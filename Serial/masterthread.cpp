﻿
#include "masterthread.h"

#include <QtSerialPort/QSerialPort>

#include <QTime>

QT_USE_NAMESPACE

MasterThread::MasterThread(QObject *parent)
    : QThread(parent), waitTimeout(0), quit(false)
{
}

//! [0]
MasterThread::~MasterThread()
{
    mutex.lock();
    quit = true;
    cond.wakeOne();
    mutex.unlock();
    wait();
}
//! [0]

//! [1] //! [2]
void MasterThread::transaction(const QString &portName, int waitTimeout, const QString &request)
{
    //! [1]
    QMutexLocker locker(&mutex);
    this->portName = portName;
    this->waitTimeout = waitTimeout;
    this->request = request;
    //! [3]
    if (!isRunning())
        start();
    //else
        //cond.wakeOne();
}
//! [2] //! [3]

//! [4]
void MasterThread::run()
{
    bool currentPortNameChanged = false;

    mutex.lock();
    //! [4] //! [5]
    QString currentPortName;
    if (currentPortName != portName) {
        currentPortName = portName;
        currentPortNameChanged = true;
    }

    int currentWaitTimeout = waitTimeout;
    QString currentRequest = request;
    mutex.unlock();
    //! [5] //! [6]
    QSerialPort serial;

    while (!quit) {
        //![6] //! [7]
        if (currentPortNameChanged) {
            serial.close();
            serial.setPortName(currentPortName);

            if (!serial.open(QIODevice::ReadWrite)) {
                emit error(tr("Can't open %1, error code %2")
                           .arg(portName).arg(serial.error()));
                return;
            }
        }
        //! [7] //! [8]
        // write request
        //QByteArray requestData = currentRequest.toLocal8Bit();

		////////////////////////////////////////////////////////////////////////////////

		DataMove.Speed = 1;
		DataMove.Direction = 2;
		DataMove.lightpower = 3;
		DataMove.Deepset = 4;
		DataMove.Roll = 5;
		DataMove.Yaw = 6;
		QByteArray Code = QByteArray::fromRawData((char*)&DataMove, sizeof(DataMove));

		//sData.Head = 0xAA;
		//sData.PackageLength = sizeof(sData); //结构体字节补齐
		////sData.PackageLength = sizeof(sData);
		//sData.SendID = 0xFE;
		//sData.ReceivedID = 0x01;
		//sData.Cmd = 0x01;
		////memcpy(&(sData.Code), &DataMove, sizeof(DataMove));
		//sData.Code = 0;
		//sData.Tail = 0xBB;

		QByteArray requestData;
		//requestData.resize(sizeof(sData));
		requestData[0] = 0xAA;
		requestData[1] = sizeof(DataMove) + 6;
		requestData[2] = 0xFE;
		requestData[3] = 0x01;
		requestData[4] = 0x00;
		requestData += Code;
		requestData += 0xBB;
		//requestData = QByteArray::fromRawData((char*)&sData, sizeof(sData));
		//////////////////////////////////////////////////////////////////////////////////

        serial.write(requestData);
        if (serial.waitForBytesWritten(waitTimeout)) {
            //! [8] //! [10]
            // read response
            if (serial.waitForReadyRead(currentWaitTimeout)) {
                QByteArray responseData = serial.readAll();
                while (serial.waitForReadyRead(10))
                    responseData += serial.readAll();

                //QString response(responseData);
                //! [12]
                emit this->response(responseData);
                //! [10] //! [11] //! [12]
            } else {
                emit timeout(tr("Wait read response timeout %1")
                             .arg(QTime::currentTime().toString()));
            }
            //! [9] //! [11]
        } else {
            emit timeout(tr("Wait write request timeout %1")
                         .arg(QTime::currentTime().toString()));
        }
        //! [9]  //! [13]
        mutex.lock();
        //cond.wait(&mutex);
        if (currentPortName != portName) {
            currentPortName = portName;
            currentPortNameChanged = true;
        } else {
            currentPortNameChanged = false;
        }
        currentWaitTimeout = waitTimeout;
        currentRequest = request;
        mutex.unlock();
    }
    //! [13]
}
