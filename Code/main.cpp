#include "PDE.h"
#include "IO.h"
#include<iostream>
#include<fstream>
#include<math.h>
#include<vector>
#include"model.h"
#include<QDir>

using namespace std;

int main()
{

	DataIO data;
	if (!data.read()) return 0;
	Model* model = data.getModel();
	//输入已知数据
	//枪膛构造及弹丸诸元
	double S = model->getArea()/1e6;   //枪膛截面积 m2
	double V0 = model->V0; //药室容积 m3
	double maxx = 0, minx = 0;
	model->getLength(minx, maxx);
	double lg = (maxx - minx)/1000;  //弹丸行程
	double m = model->m; //弹丸质量 kg
	//装药条件
	double f = model->f;       //火药力 KJ/Kg
	double a = model->a;    //余容
	double w = model->w;       //装药量 g
	double Pp = model->Pp;    //火药密度 Kg/m3
	double c = model->c;     //火药热力系数
	double u1 = model->u1; //燃速系数
	double n = model->n;     //压力指数
	double e1 = model->e1;    //弧厚 m
	double cai = model->cai;  //形状特征量
	double r = model->r;
	double u = model->u;
	double p = model->p;
	//初始条件
	double p0 = model->p0;     //MPa
	//次要功计算参数
	double K1 = model->K1;
	double b = model->b;
	//计算步长
	double h = model->h;
	//常量计算
	double Zk = (p + e1) / e1;
	double fai = K1 + b*w / (m * 1000);   //次要功系数
	double de = w / (V0*1E6);        //装填密度
	double l0 = V0 / S;             //自由容积缩径长 m
	double Vj = sqrt((2 * f*w) / (c*fai*m));  //理论最大速度 m/s
	double B = ((S*S*e1*e1) / (f*w*fai*m*u1*u1))*pow(f*de, 2 * (1 - n));
	double Ks = cai*(1 + r + u);
	double cais = (1 - Ks*Zk*Zk) / (Zk - Zk*Zk);
	double rs = Ks / cais - 1;

	//初值计算
	Point temp;
	vector<Point> pt;
	double tb = 0;
	double lb = 0;
	double vb = 0;
	double pb = p0 / (f*de);
	double K = (1 / de - 1 / Pp) / (f / p0 + a - 1 / Pp);
	double c0 = sqrt(1 + 4 * r*K / cai);
	double Z = (c0 - 1) / (2 * r);
	temp.t = tb;
	temp.l = lb;
	temp.v = vb;
	temp.p = pb;
	temp.Z = Z;
	temp.K = K;
	pt.push_back(temp);
	double end = lg / l0;
	vector<Point>::iterator iter;
	iter = pt.end() - 1;
	while ((*iter).l <= end)
	{
		temp = Runge_Kutta(*iter, h, Zk, c, B, n, cai, r, u, cais, rs, de, Pp, a);
		pt.push_back(temp);
		iter = pt.end() - 1;
	}
	for (iter = pt.begin(); iter != pt.end(); iter++)
	{
		*iter = Return(*iter, l0, Vj, f, de);
	}
	QString csp = QDir::currentPath();
	QDir mdir(csp + "/MonitorFiles");
	if (mdir.exists())
		mdir.removeRecursively();
	mdir.mkdir(csp +"/MonitorFiles");

	mdir.setPath( csp + "/Result");
	if (mdir.exists())
		mdir.removeRecursively();
	mdir.mkdir(csp +"/Result");

	ofstream outfile("MonitorFiles/realTime.dat", ios::out);
	outfile << "t(s)" << "  "  << "K" << endl;
	outfile.close();

	ofstream resfile("Result/res.dat", ios::out);
	resfile << "t(s)" << "  " << "l(m)" << "  " << "v(m/s)" << "  " << "p(MPa)" << "  " << "K" << "  " << "Z" << endl;
	
	int step = 0;
	for (iter = pt.begin(); iter != pt.end(); iter++)
	{
		ofstream reaFile("MonitorFiles/realTime.dat", ios::app);
		reaFile << (*iter).t<<"  "<<(*iter).K << endl;
		reaFile.close();
		resfile << (*iter).t << "  " << (*iter).l << "  " << (*iter).v << "  " << (*iter).p << "  " << (*iter).K << "  " << (*iter).Z << endl;

		data.write(step,*iter);
		++step;
	}
	resfile.close();
	
	cout << "内弹道求解完毕！" << endl;
}
