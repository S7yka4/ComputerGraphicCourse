#include "Render.h"




#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "MyShaders.h"

#include "ObjLoader.h"
#include "GUItextRectangle.h"

#include "Texture.h"

GuiTextRectangle rec;
const double MathPI = 3.141592653589793;
bool textureMode = true;
bool lightMode = true;
bool buttoncheck = false;
//double t_max = 0;

//небольшой дефайн для упрощения кода
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile *model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //массивчик для десяти шейдеров
Shader frac;
Shader cassini;




//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		/*camDist = 15;//default
		fi1 = 1;
		fi2 = 1;*/
		camDist = 30;
			fi1 = 0.9;
			fi2 = 0;

	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	virtual void SetUpCamera()
	{

		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//класс недоделан!
class WASDcamera :public CustomCamera
{
public:
		
	float camSpeed;

	WASDcamera()
	{
		camSpeed = 0.4;
		pos.setCoords(5, 5, 5);
		lookPoint.setCoords(0, 0, 0);
		normal.setCoords(0, 0, 1);
	}

	virtual void SetUpCamera()
	{

		if (OpenGL::isKeyPressed('W'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*camSpeed;
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}
		if (OpenGL::isKeyPressed('S'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*(-camSpeed);
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}

		LookAt();
	}

} WASDcam;


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);
		
		glDisable(GL_DEPTH_TEST);
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
				glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}
		/*
		if (f1)
			glEnable(GL_LIGHTING);
		if (f2)
			glEnable(GL_TEXTURE_2D);
		if (f3)
			glEnable(GL_DEPTH_TEST);
			*/
	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света



//старые координаты мыши
int mouseX = 0, mouseY = 0;


double t_max2 = 0;
double t_max3 = 0;
double t_max = 0;
double angle3;
double angle2;
double angle0;
double angle1 = 0;
float offsetX = 0, offsetY = 0;
float zoom=1;
float Time = 0;
int tick_o = 0;
double savep[3];
int tick_n = 0;

//обработчик движения мыши
void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}


	if (OpenGL::isKeyPressed(VK_LBUTTON))
	{
		offsetX -= 1.0*dx/ogl->getWidth()/zoom;
		offsetY += 1.0*dy/ogl->getHeight()/zoom;
	}


	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y,60,ogl->aspect);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

//обработчик вращения колеса  мыши
void mouseWheelEvent(OpenGL *ogl, int delta)
{


	float _tmpZ = delta*0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2*zoom*_tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;
}

//обработчик нажатия кнопок клавиатуры
void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}
	if ((OpenGL::isKeyPressed('K'))&&(!buttoncheck))
		buttoncheck = true;
	else
		if ((OpenGL::isKeyPressed('K')) && (buttoncheck))
		{
			buttoncheck = false;
			savep[0] = 0;
			savep[1] = 0;
			savep[2] = 0;
			angle3 = 0;
			angle2 = 0;
			angle1 = 0;
			angle0 = 0;
		}
	if (key == 'T')
	{
		textureMode = !textureMode;
	}	   

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}

	if (key == 'S')
	{
		frac.LoadShaderFromFile();
		frac.Compile();

		s[0].LoadShaderFromFile();
		s[0].Compile();

		cassini.LoadShaderFromFile();
		cassini.Compile();
	}

	if (key == 'Q')
		Time = 0;
}

void keyUpEvent(OpenGL *ogl, int key)
{

}


void DrawQuad()
{
	double A[] = { 0,0 };
	double B[] = { 1,0 };
	double C[] = { 1,1 };
	double D[] = { 0,1 };
	glBegin(GL_QUADS);
	glColor3d(.5, 0, 0);
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);
	glEnd();
}


ObjFile objModel,monkey, Body, Hand1, Hand2, RightLoc, LeftLoc,Head,Eye1,Eye2;

Texture TBody,Thand1,Thand2,TRightLoc,TLeftLoc, THead, TEye1, TEye2;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{

	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	
	


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	/*
	//texture1.loadTextureFromFile("textures\\texture.bmp");   загрузка текстуры из файла
	*/


	frac.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	frac.FshaderFileName = "shaders\\frac.frag"; //имя файла фрагментного шейдера
	frac.LoadShaderFromFile(); //загружаем шейдеры из файла
	frac.Compile(); //компилируем

	cassini.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	cassini.FshaderFileName = "shaders\\cassini.frag"; //имя файла фрагментного шейдера
	cassini.LoadShaderFromFile(); //загружаем шейдеры из файла
	cassini.Compile(); //компилируем
	

	s[0].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[0].FshaderFileName = "shaders\\light.frag"; //имя файла фрагментного шейдера
	s[0].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[0].Compile(); //компилируем

	s[1].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //имя файла фрагментного шейдера
	s[1].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[1].Compile(); //компилируем

	

	 //так как гит игнорит модели *.obj файлы, так как они совпадают по расширению с объектными файлами, 
	 // создающимися во время компиляции, я переименовал модели в *.obj_m
	//loadModel("models\\lpgun6.obj_m", &objModel);


	//glActiveTexture(GL_TEXTURE0);
	//loadModel("models\\monkey.obj_m", &monkey);
	//monkeyTex.loadTextureFromFile("textures//tex.bmp");
	//monkeyTex.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("objects\\Securitronbody2.obj_m", &Body);
	TBody.loadTextureFromFile("textures//body2.bmp");
	TBody.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("objects\\Head.obj_m", &Head);
	THead.loadTextureFromFile("textures//Head.bmp");
	THead.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("objects\\Plecho3.obj_m", &LeftLoc);
	TLeftLoc.loadTextureFromFile("textures//hand.bmp");
	TLeftLoc.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("objects\\Plecho3.obj_m", &RightLoc);
	TRightLoc.loadTextureFromFile("textures//hand.bmp");
	TRightLoc.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("objects\\Hand3.obj_m", &Hand2);
	Thand2.loadTextureFromFile("textures//Thand.bmp");
	Thand2.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("objects\\Hand3.obj_m", &Hand1);
	Thand1.loadTextureFromFile("textures//Thand.bmp");
	Thand1.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("objects\\eyeLeft.obj_m", &Eye1);
	TEye1.loadTextureFromFile("textures//body.bmp");
	TEye1.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("objects\\eyeRight.obj_m", &Eye2);
	TEye2.loadTextureFromFile("textures//body.bmp");
	TEye2.bindTexture();


	tick_n = GetTickCount();
	tick_o = tick_n;

	rec.setSize(300, 100);
	rec.setPosition(10, ogl->getHeight() - 100-10);
	rec.setText("K - воспроизведение анимации\nT - вкл/выкл текстур\nL - вкл/выкл освещение\nF - Свет из камеры\nG - двигать свет по горизонтали\nG+ЛКМ двигать свет по вертекали",0,0,0);

	
}





Vector3 PointRightHand;

double delta_time=0.01;
double k = 0.04;


bool flag=true;
void RotateHand()
{
	

	if(flag)
	{
		glPushMatrix();
		glTranslated(3 * cos((310) * M_PI / 180.0) + 2.8, -0.2, (3 * sin((310) * M_PI / 180.0) + 7.0));
		glRotated(90, 1, 0, 0);
		glRotated(angle0, 0, 0, 1);
		Thand1.bindTexture();
		Hand1.DrawObj();
		angle0 = angle0 + 2;
		glPopMatrix();
		if (angle0> 120)
			flag = false;
	}
	else
	if(!flag)
	{
		glPushMatrix();
		glTranslated(3 * cos((310) * M_PI / 180.0) + 2.8, -0.2, (3 * sin((310) * M_PI / 180.0) + 7.0));
		glRotated(90, 1, 0, 0);
		glRotated(angle0, 0, 0, 1);
		Thand1.bindTexture();
		Hand1.DrawObj();
		angle0= angle0 - 2;
		glPopMatrix();
		if (angle0 < 80)
			flag = true;
	}
}


void MoveHand(double delta_time)
{
	if (angle1 < 80)
	{
		glPushMatrix();
		glTranslated(3 * cos((310) * M_PI / 180.0) + 2.8, -0.2, (3 * sin((310) * M_PI / 180.0) + 7.0));
		glRotated(90, 1, 0, 0);
		glRotated(angle1, 0, 0, 1);
		Thand1.bindTexture();
		Hand1.DrawObj();
		angle1 = angle1 + 1;
		angle0 = angle1;
		glPopMatrix();
	}
	else

		RotateHand();


}



double saveangle;
double count = 0;
double count2 = 0;
double framecount = 0;

//double t = 0;



double savep2[3];



void MoveLocot2(double delta_time)
{
	double p[] = { 2.9,-0.2,4 };
	if (angle3 * 4 <40.0)
	{
		glPushMatrix();
		glTranslated(2.8, 0, 7);
		glRotated(90, 1, 0, 0);
		
		p[0] = 3 * sin((180 - angle3 * 4) * M_PI / 180.0) + 2.8;
		p[1] = -0.2;
		p[2] = 3 * cos((180 - angle3 * 4) * M_PI / 180.0) + 7.0;
		
		angle3 += 0.8;
		glRotated(angle3 * 4, 0, 0, 1);
		TRightLoc.bindTexture();
		RightLoc.DrawObj();
		glPopMatrix();
		glPushMatrix();//
		glTranslated(p[0],p[1], p[2]);//
		glRotated(90, 1, 0, 0);//
		Thand1.bindTexture();
		Hand1.DrawObj();//
		glPopMatrix();//

	}
	else
	{
		glPushMatrix();
		glTranslated(2.8, 0, 7);
		glRotated(90, 1, 0, 0);
		glRotated(angle3 * 4, 0, 0, 1);
		TRightLoc.bindTexture();
		RightLoc.DrawObj();
		glPopMatrix();
		MoveHand(delta_time);
	}
}
OpenGL *tmp;




void Render(OpenGL *ogl)
{   
	tmp = ogl;
	tick_o = tick_n;
	tick_n = GetTickCount();
	Time += (tick_n - tick_o) / 1000.0;

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	*/
	//UpdateParametr();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================
	//Прогать тут  


	s[2].UseShader();
	int l = glGetUniformLocationARB(s[1].program, "tex");
	glUniform1iARB(l, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(90,1, 0, 0);
	glTranslated(0, 0, 0);
	TBody.bindTexture();//текстура
	Body.DrawObj();//рисование модели
	glPopMatrix();


	
	if(!buttoncheck)
	{ 
	s[3].UseShader();
    int l3 = glGetUniformLocationARB(s[3].program, "tex");
	glUniform1iARB(l3, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	
	glRotated(90, 1, 0, 0);
	glTranslated(2.8, 4, 0);
	//glRotated(180, 0, 1, 0);
	Thand1.bindTexture();
	Hand1.DrawObj();//рисование модели
	glPopMatrix();
	
	s[5].UseShader();
	int l5 = glGetUniformLocationARB(s[5].program, "tex");
	glUniform1iARB(l5, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	glTranslated(2.8, 7,0);

	TRightLoc.bindTexture();//текстура
	RightLoc.DrawObj();//рисование модели
	glPopMatrix();
	}
	else
	{
		MoveLocot2(delta_time);
	}
	

	s[4].UseShader();
	int l4 = glGetUniformLocationARB(s[4].program, "tex");
	glUniform1iARB(l4, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	glTranslated(-2.9, 4, 0);
	glRotated(180, 0, 1, 0);
	Thand2.bindTexture();//текстура
	Hand2.DrawObj();//рисование модели
	glPopMatrix();



	s[6].UseShader();
	int l6 = glGetUniformLocationARB(s[6].program, "tex");
	glUniform1iARB(l6, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	glTranslated(-2.8, 7, 0);
	TLeftLoc.bindTexture();//текстура
	LeftLoc.DrawObj();//рисование модели
	glPopMatrix();

	s[7].UseShader();
	int l7 = glGetUniformLocationARB(s[7].program, "tex");
	glUniform1iARB(l7, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	glTranslated(0, 0, 0);
	THead.bindTexture();//текстура
	Head.DrawObj();//рисование модели
	glPopMatrix();

	s[8].UseShader();
	int l8 = glGetUniformLocationARB(s[8].program, "tex");
	glUniform1iARB(l8, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	glTranslated(0, 0, 0);
	TEye1.bindTexture();//текстура
	Eye1.DrawObj();//рисование модели
	glPopMatrix();

	s[9].UseShader();
	int l9 = glGetUniformLocationARB(s[9].program, "tex");
	glUniform1iARB(l9, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	glTranslated(0, 0, 0);
	TEye2.bindTexture();//текстура
	Eye2.DrawObj();//рисование модели
	glPopMatrix();

	bool flag = false;






	s[0].UseShader();

	//передача параметров в шейдер.  Шаг один - ищем адрес uniform переменной по ее имени. 
	int location = glGetUniformLocationARB(s[0].program, "light_pos");
	//Шаг 2 - передаем ей значение
	glUniform3fARB(location, light.pos.X(), light.pos.Y(),light.pos.Z());

	location = glGetUniformLocationARB(s[0].program, "Ia");
	glUniform3fARB(location, 0.2, 0.2, 0.2);

	location = glGetUniformLocationARB(s[0].program, "Id");
	glUniform3fARB(location, 1.0, 1.0, 1.0);

	location = glGetUniformLocationARB(s[0].program, "Is");
	glUniform3fARB(location, .7, .7, .7);


	location = glGetUniformLocationARB(s[0].program, "ma");
	glUniform3fARB(location, 0.2, 0.2, 0.1);

	location = glGetUniformLocationARB(s[0].program, "md");
	glUniform3fARB(location, 0.4, 0.65, 0.5);

	location = glGetUniformLocationARB(s[0].program, "ms");
	glUniform4fARB(location, 0.9, 0.8, 0.3, 25.6);

	location = glGetUniformLocationARB(s[0].program, "camera");
	glUniform3fARB(location, camera.pos.X(), camera.pos.Y(), camera.pos.Z());




	
	

	//////Рисование фрактала

	
	/*
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,1,0,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		frac.UseShader();

		int location = glGetUniformLocationARB(frac.program, "size");
		glUniform2fARB(location, (GLfloat)ogl->getWidth(), (GLfloat)ogl->getHeight());

		location = glGetUniformLocationARB(frac.program, "uOffset");
		glUniform2fARB(location, offsetX, offsetY);

		location = glGetUniformLocationARB(frac.program, "uZoom");
		glUniform1fARB(location, zoom);

		location = glGetUniformLocationARB(frac.program, "Time");
		glUniform1fARB(location, Time);

		DrawQuad();

	}
	*/
	
	
	//////Овал Кассини
	
	/*
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,1,0,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		cassini.UseShader();

		int location = glGetUniformLocationARB(cassini.program, "size");
		glUniform2fARB(location, (GLfloat)ogl->getWidth(), (GLfloat)ogl->getHeight());


		location = glGetUniformLocationARB(cassini.program, "Time");
		glUniform1fARB(location, Time);

		DrawQuad();
	}

	*/

	
	
	

	
	Shader::DontUseShaders();

	
	
}   //конец тела функции


bool gui_init = false;

//рисует интерфейс, вызывется после обычного рендера
void RenderGUI(OpenGL *ogl)
{
	
	Shader::DontUseShaders();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	

	glActiveTexture(GL_TEXTURE0);
	rec.Draw();


		
	Shader::DontUseShaders(); 



	
}

void resizeEvent(OpenGL *ogl, int newW, int newH)
{
	rec.setPosition(10, newH - 100 - 10);
}

