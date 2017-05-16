#include "AHRSEKF.h"
#include <iostream>
#include <fstream>

namespace RAIN_IMU
{

AHRSEKF::AHRSEKF()
{
}

AHRSEKF::~AHRSEKF()
{
}

/**
 *  euler[0] euler[1] euler[2]
 *  yaw		pitch	 roll
 *
**/
Eigen::Vector3d AHRSEKF::Initialize(const SensorData &sensordata)
{
	double pitch, roll, yaw;

	pitch = atan2(sensordata.Acc.X, sqrt(sensordata.Acc.Y*sensordata.Acc.Y + sensordata.Acc.Z*sensordata.Acc.Z));
	roll = atan2(-sensordata.Acc.Y, -sensordata.Acc.Z);
	
	double r1 = -sensordata.Mag.Y*cos(roll) + sensordata.Mag.Z*sin(roll);
	double r2 = sensordata.Mag.X*cos(pitch) + sensordata.Mag.Y*sin(pitch)*sin(roll) + sensordata.Mag.Z*sin(pitch)*cos(roll);

	yaw = atan2(r1, r2) - 8.3 * DEG_RAD;

	return Eigen::Vector3d(yaw, pitch, roll);
}

Eigen::Matrix<double, 4, 4> AHRSEKF::DiscreteTime(const Eigen::Matrix<double, 4, 4> &rotM, const double &T)
{
	Eigen::Matrix<double, 4, 4> r1 = 0.5 * T * rotM;

	return (Eigen::MatrixXd::Identity(3, 3) + r1);

}

Eigen::Matrix<double, 4, 4> AHRSEKF::Gyro2RotationalMatrix(const SensorData &sensordata)
{
	Eigen::Matrix<double, 4, 4> rotM;

	rotM << 0, -sensordata.Acc.X, -sensordata.Acc.Y, -sensordata.Acc.Z,
			sensordata.Acc.X, 0, sensordata.Acc.Z, -sensordata.Acc.Y,
			sensordata.Acc.Y, -sensordata.Acc.Z, 0, sensordata.Acc.X,
			sensordata.Acc.Z, sensordata.Acc.Y, -sensordata.Acc.X, 0;
	
	return rotM;
}

/**
 * �����ݼ���27 28 29 �ֱ��ǽ��ٶȼƵ�x��y��z������
 *			  9 10 11 �ֱ��Ǽ��ٶȼƵ�x��y��z������
 *			 15 16 17 �ֱ��Ǵ����̵�x��y��z������
 *			 30 31 32 �ֱ��ǽ�����̬��ground truth��Roll��Pitch��Yaw

**/
void AHRSEKF::ReadSensorData()
{
	std::cout << "read the sensor raw data" << std::endl;

	const unsigned int ROW = 36, VOL = 1000;
	double d[VOL][ROW];
	std::ifstream in("myfile.txt");
	for (int i = 0; i < VOL; i++)
	{
		for (int j = 0; j < ROW; j++)
		{
			in >> d[i][j];
		}
	}	
	in.close();

	SensorData sensordata;
	for (int i = 0; i <	VOL; i++)
	{
		sensordata.nId = i;

		sensordata.Acc.X = d[i][8];
		sensordata.Acc.Y = d[i][9];
		sensordata.Acc.Z = d[i][10];

		sensordata.Gyro.X = d[i][26];
		sensordata.Gyro.Y = d[i][27];
		sensordata.Gyro.Z = d[i][28];

		sensordata.Mag.X = d[i][14];
		sensordata.Mag.Y = d[i][15];
		sensordata.Mag.Z = d[i][16];

		sensordata.EulerGroundTruth.Roll = d[i][29];
		sensordata.EulerGroundTruth.Pitch = d[i][30];
		sensordata.EulerGroundTruth.Yaw = d[i][31];

		vSensorData.push_back(sensordata);
	}

	//std::cout.precision(10);
	//for (int i = 0; i < 100;i++)
	//{
	//	std::cout << vSensorData.at(i).Mag.X << std::endl;
	//}

	std::cout << "finish loading the dataset" << std::endl;
}

SensorData AHRSEKF::GetSensordatabyID(long unsigned int nId)
{
	return vSensorData.at(nId);
}

}