#include "mui.h"

MUI::MUI(QWidget *parent)
	: QWidget(parent),
	m_realTime(0.0f),
	m_timerId(0)
{
	ui.setupUi(this);
	timer = new QTimer(this);
	timer->start(100);
	m_timerId = startTimer(1000);
	//connect(timer, SIGNAL(timeout()), this, SLOT(update()));

	////////////////////////////////////////////////////
	const auto infos = QSerialPortInfo::availablePorts();
	for (const QSerialPortInfo &info : infos)
		ui.serialPortComboBox->addItem(info.portName());

	//connect(ui.runButton, &QPushButton::clicked, this, &MUI::transaction);
	connect(timer, &QTimer::timeout, this, &MUI::transaction);
	connect(ui.runButton, &QPushButton::clicked, this, &MUI::transaction);
	connect(&thread, &MasterThread::response, this, &MUI::showResponse);
	connect(&thread, &MasterThread::error, this, &MUI::processError);
	connect(&thread, &MasterThread::timeout, this, &MUI::processTimeout);
	////////////////////////////////////////////////////

	m_time.start();
}

MUI::~MUI()
{
	if (m_timerId) killTimer(m_timerId);

}

void MUI::timerEvent(QTimerEvent * event)
{
	/////////////////////////////////
	QWidget::timerEvent(event);
	/////////////////////////////////

	//float timeStep = m_time.restart();
	//m_realTime = m_realTime + timeStep / 1000.0f;

	//float roll = 0.0f;
	//roll = 180 * sin(m_realTime / 5.0f);
	//ui.graphicsMUI->setRoll(roll);
	

	//float pitch = 0.0f;
	//pitch = 90.0f * sin(m_realTime / 40.0f) + 90.0f;
	//ui.graphicsMUI->setPitch(DataStateAck.pitch);

	

}

//void MUI::update()
//{
	//ui.graphicsMUI->update();
//}

void MUI::transaction()
{
	setControlsEnabled(false);
	ui.statusLabel->setText(tr("Status: Running, connected to port %1.")
		.arg(ui.serialPortComboBox->currentText()));
	switch (ui.sendCMDBox->currentText().toInt())
	{
	case 0:
		setRequestData(CMD_MOVE);
		break;
	case 1:
		setRequestData(CMD_STATE);
		break;
	case 2:
		setRequestData(CMD_FORMATE);
		break;
	case 3:
		setRequestData(CMD_LASERCONTROL);
		break;
	case 4:
		setRequestData(CMD_PICTUREINF);
		break;
	case 5:
		setRequestData(CMD_GPSCOORD);
		break;
	case 6:
		setRequestData(CMD_SOUNDCONTROL);
		break;
	case 7:
		setRequestData(CMD_DOWNLOADCONTROL);
		break;
	default:
		break;
	}
	//setRequestData(CMD_MOVE);

	thread.transaction(
		ui.serialPortComboBox->currentText(),
		ui.waitResponseSpinBox->value(),
		requestData);
}

void MUI::showResponse(const QByteArray & s)
{
	setControlsEnabled(true);

	//添加解析函数，将QByteArray反序列化。
	//const DataPackage_TypeDef* pData = (DataPackage_TypeDef*)(s.data());

	//memcpy(&DataMove, &(pData->Code), sizeof(DataMove));  //数字11 = 12 - 1字节对齐
	//if (QString(s.toHex()).left(2)== "aa" && QString(s.toHex()).right(2) == "bb")
	{
		//memcpy(&DataMove, s.data()+5, s[1] - 6);//截取字节段copy到结构体。
		getResponseData(s);
		//ui.showDate->setText(s.toHex(':'));
		/*ui.trafficLabel->setText(
		tr(	" Speed       %1"
		"\n\rDirection:  %2"
		"\n\rlightpower: %3")
		.arg(QString::number(DataStateAck.speed))
		.arg(QString::number(DataStateAck.Direction))
		.arg(QString::number(DataStateAck.Power)));*/

		ui.trafficLabel->setText(tr(
			"\n\r-DatastateAck.speed	 : %1"
			"\n\r-DatastateAck.Direction : %2"
			"\n\r-DatastateAck.Power     : %3"
			"\n\r-DatastateAck.Temp      : %4"
			"\n\r-DatastateAck.Hum		 : %5"
			"\n\r-DatastateAck.Roll		 : %6"
			"\n\r-DatastateAck.pitch	 : %7"
			"\n\r-DatastateAck.Yaw		 : %8"
			"\n\r-DatastateAck.pointID	 : %9"
			"\n\r-DatastateAck.v         : %10"
			"\n\r-DatastateAck.NowDeepth : %11"
			"\n\r-DatastateAck.point.x	 : %12"
			"\n\r-DatastateAck.point.y   : %13"
		)
			.arg((int)(DataStateAck.speed))
			.arg((int)(DataStateAck.Direction))
			.arg((int)(DataStateAck.Power))
			.arg((int)(DataStateAck.Temp))
			.arg((int)(DataStateAck.Hum))
			.arg((int)(DataStateAck.Roll))
			.arg((int)(DataStateAck.pitch))
			.arg((int)(DataStateAck.Yaw))
			.arg((int)(DataStateAck.pointID))
			.arg((int)(DataStateAck.v))
			.arg((int)(DataStateAck.NowDeepth))
			.arg((int)(DataStateAck.point.x))
			.arg((int)(DataStateAck.point.y))
		);

		ui.graphicsMUI->setRoll((int)DataStateAck.Roll);
		ui.graphicsMUI->setPitch((int)DataStateAck.pitch);
		ui.graphicsMUI->update();
	}
}

void MUI::processError(const QString & s)
{
	setControlsEnabled(true);
	//ui.statusLabel->setText(tr("Status: Not running, %1.").arg(s));
	//ui.trafficLabel->setText(tr("No traffic."));
}

void MUI::processTimeout(const QString & s)
{
	setControlsEnabled(true);
	//ui.statusLabel->setText(tr("Status: Running, %1.").arg(s));
	//ui.trafficLabel->setText(tr("No traffic."));
}

void MUI::setControlsEnabled(bool enable)
{
	ui.runButton->setEnabled(enable);
	ui.serialPortComboBox->setEnabled(enable);
	ui.waitResponseSpinBox->setEnabled(enable);
	//ui.requestLineEdit->setEnabled(enable);
}

void MUI::setRequestData(TrainCmd cmd)
{
	QByteArray Code;  //
	requestData.clear();
	requestData[0] = 0xAA;
	requestData[2] = 0xFE;
	requestData[3] = 0x01;
	switch (cmd)
	{
	case CMD_MOVE: //0
		Code = QByteArray::fromRawData((char*)&DataMove, sizeof(DataMove));
		requestData[1] = sizeof(DataMove) + 6;
		requestData[4] = 0x00;
		break;
	case CMD_STATE:
		requestData[1] = 1 + 6;
		requestData[4] = 0x01;
		Code[0] = 0x00;
		break;
	case CMD_FORMATE:
		requestData[1] = 1 + 6;
		requestData[4] = 0x02;
		Code[0] = 0x00;
		break;
	case CMD_LASERCONTROL:
		requestData[1] = 1 + 6;
		requestData[4] = 0x03;
		Code[0] = 0x00;
		break;
	case CMD_PICTUREINF:
		requestData[1] = 1 + 6;
		requestData[4] = 0x04;
		Code[0] = 0x00;
		break;
	case CMD_GPSCOORD:
		requestData[1] = 1 + 6;
		requestData[4] = 0x05;
		Code[0] = 0x00;
		break;
	case CMD_SOUNDCONTROL:
		requestData[1] = 1 + 6;
		requestData[4] = 0x06;
		Code[0] = 0x00;
		break;
	case CMD_DOWNLOADCONTROL:
		requestData[1] = 1 + 6;
		requestData[4] = 0x07;
		Code[0] = 0x00;
		break;
	default:
		break;
	}
	requestData += Code;
	requestData += 0xBB;

	//ui.requestLineEdit->setText(requestData.toHex(':'));
}

void MUI::getResponseData(const QByteArray & s)
{
	//if (QString(s.toHex()).left(2) == "aa" && QString(s.toHex()).right(2) == "bb")
	if (s[0] == (char)0xaa && s[s.size() - 1] == (char)0xbb)
	{
		switch (s[4])
		{
		case CMD_MOVE: //0
			memcpy(&DataMove, s.data() + 5, s[1] - 6);//截取字节段copy到结构体。
			break;
		case CMD_STATE:
			memcpy(&DataStateAck, s.data() + 5, s[1] - 6);//截取字节段copy到结构体。
			break;
		case CMD_FORMATE:

			break;
		case CMD_LASERCONTROL:

			break;
		case CMD_PICTUREINF:

			break;
		case CMD_GPSCOORD:

			break;
		case CMD_SOUNDCONTROL:

			break;
		case CMD_DOWNLOADCONTROL:

			break;
		default:
			break;
		}
	}
}
