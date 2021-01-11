#include "myopengl.h"
#include <QDebug>
#include <QCoreApplication>
#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QDebug>
#include <GL/GLU.h>
#include <QGLWidget>

MyOpenGL::MyOpenGL(QWindow *parent):
    QWindow(parent)
    , m_context(0)
    , m_yrot(0.0f)
    , m_xpos(0.0f)
    , m_zpos(0.0f)
    , m_heading(0.0f)
    , m_walkbias(0.0f)
    , m_walkbiasangle(0.0f)
    , m_lookupdown(0.0f)
    , m_filter(0)
    ,fullScreen(false)
{
    setSurfaceType(QWindow::OpenGLSurface);
}

MyOpenGL::~MyOpenGL()
{
    glDeleteTextures(1, &m_texture[0]);
}

void MyOpenGL::render(QPainter *painter)
{
    Q_UNUSED(painter);
}
//显示世界
void MyOpenGL::render()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glViewport(0,0,(GLint)width(),(GLint)height()); // 重置当前视口
    glMatrixMode(GL_PROJECTION);                    // 选择投影矩阵
    glLoadIdentity();                               // 重置投影矩阵为单位矩阵
    gluPerspective(45.0f,(GLdouble)width()/(GLdouble)height(),0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);// 选择模型视图矩阵
    glLoadIdentity();          // 重置模型视图矩阵为单位矩阵

    GLfloat x_m, y_m, z_m, u_m, v_m;        // 顶点的临时 X, Y, Z, U 和 V 的数值
    GLfloat xtrans = -m_xpos;				// 用于游戏者沿X轴平移时的大小
    GLfloat ztrans = -m_zpos;				// 用于游戏者沿Z轴平移时的大小
    GLfloat ytrans = -m_walkbias-0.25f;		// 用于头部的上下摆动
    GLfloat sceneroty = 360.0f - m_yrot;	// 位于游戏者方向的360度角
    int numtriangles;						// 保有三角形数量的整数
    glRotatef(m_lookupdown, 1.0f, 0 ,0);    // 上下旋转
    glRotatef(sceneroty, 0, 1.0f, 0);		// 左右旋转
    glTranslatef(xtrans, ytrans, ztrans);	// 以游戏者为中心的平移场景
    //glBindTexture(GL_TEXTURE_2D, m_texture[m_filter]);	   // 根据filter选择的纹理
    numtriangles = m_sector1.numtriangles;				   // 取得Sector1的三角形数量
    for (int loop_m = 0; loop_m < numtriangles; loop_m++)  // 遍历所有的三角形
    {
        if(loop_m<2)
            glBindTexture(GL_TEXTURE_2D, m_texture[0]);
        else if(loop_m<4)
            glBindTexture(GL_TEXTURE_2D, m_texture[1]);
        else
            glBindTexture(GL_TEXTURE_2D, m_texture[2]);
        glBegin(GL_TRIANGLES);					        // 开始绘制三角形
        x_m = m_sector1.triangle[loop_m].vertex[0].x-0.5;	// 第一点的 X 分量
        y_m = m_sector1.triangle[loop_m].vertex[0].y;	// 第一点的 Y 分量
        z_m = m_sector1.triangle[loop_m].vertex[0].z;	// 第一点的 Z 分量
        u_m = m_sector1.triangle[loop_m].vertex[0].u;	// 第一点的 U  纹理坐标
        v_m = m_sector1.triangle[loop_m].vertex[0].v;	// 第一点的 V  纹理坐标

        glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);	// 设置纹理坐标和顶点
        x_m = m_sector1.triangle[loop_m].vertex[1].x-0.5;	// 第二点的 X 分量
        y_m = m_sector1.triangle[loop_m].vertex[1].y;	// 第二点的 Y 分量
        z_m = m_sector1.triangle[loop_m].vertex[1].z;	// 第二点的 Z 分量
        u_m = m_sector1.triangle[loop_m].vertex[1].u;	// 第二点的 U  纹理坐标
        v_m = m_sector1.triangle[loop_m].vertex[1].v;	// 第二点的 V  纹理坐标

        glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);	// 设置纹理坐标和顶点
        x_m = m_sector1.triangle[loop_m].vertex[2].x-0.5;	// 第三点的 X 分量
        y_m = m_sector1.triangle[loop_m].vertex[2].y;	// 第三点的 Y 分量
        z_m = m_sector1.triangle[loop_m].vertex[2].z;	// 第三点的 Z 分量
        u_m = m_sector1.triangle[loop_m].vertex[2].u;	// 第二点的 U  纹理坐标
        v_m = m_sector1.triangle[loop_m].vertex[2].v;	// 第二点的 V  纹理坐标
        glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);	// 设置纹理坐标和顶点
        glEnd();						                // 三角形绘制结束
    }
}

void MyOpenGL::initialize()
{
    loadGLTexture();                      // 加载纹理
    glEnable(GL_TEXTURE_2D);              // 启用纹理映射
    glShadeModel(GL_SMOOTH);              // 启用平滑着色
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 黑色背景
    glClearDepth(1.0f);                   // 设置深度缓存
    glEnable(GL_DEPTH_TEST);              // 启用深度测试
    glDepthFunc(GL_LEQUAL);               // 深度测试类型
    // 透视修正
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    setupWorld();
}

void MyOpenGL::renderNow()
{
    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    render();

    m_context->swapBuffers(this);
}


//载入文件
void MyOpenGL::setupWorld()
{
    QFile file("E:/QT_file/Graphics_Project3/World3.txt");
    if(!file.exists())
    {
        qDebug()<<"file does not exist!";
        return;
    }
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"Can't open world file.";
        return;
    }

    QTextStream stream(&file);
    //对区段进行初始化，并读入部分数据
    QString oneline;		// 存储数据的字符串
    int numtriangles;		// 区段的三角形数量
    float x, y, z, u, v;	// 3D 和 纹理坐标

    readStr(&stream, oneline); // 读入一行数据
    sscanf(oneline.toLatin1().data(), "NUMPOLLIES %d\n", &numtriangles); // 读入三角形数量

    m_sector1.triangle = new TRIANGLE[numtriangles];		 // 为numtriangles个三角形分配内存并设定指针
    m_sector1.numtriangles = numtriangles;					 // 定义区段1中的三角形数量
    // 遍历区段中的每个三角形
    for (int triloop = 0; triloop < numtriangles; triloop++) // 遍历所有的三角形
    {
        float temy;
        bool iswall=false;
        LINE line;
        // 遍历三角形的每个顶点
        for (int vertloop = 0; vertloop < 3; vertloop++)	 // 遍历所有的顶点
        {
            readStr(&stream, oneline);				         // 读入一行数据
            // 读入各自的顶点数据
            sscanf(oneline.toLatin1().data(), "%f %f %f %f %f", &x, &y, &z, &u, &v);
            // 将顶点数据存入各自的顶点
            m_sector1.triangle[triloop].vertex[vertloop].x = x;	// 区段 1,  第 triloop 个三角形, 第  vertloop 个顶点, 值 x=x
            m_sector1.triangle[triloop].vertex[vertloop].y = y;	// 区段 1,  第 triloop 个三角形, 第  vertloop 个顶点, 值 y=y
            m_sector1.triangle[triloop].vertex[vertloop].z = z;	// 区段 1,  第 triloop 个三角形, 第  vertloop 个顶点, 值 z=z
            m_sector1.triangle[triloop].vertex[vertloop].u = u;	// 区段 1,  第 triloop 个三角形, 第  vertloop 个顶点, 值 u=u
            m_sector1.triangle[triloop].vertex[vertloop].v = v;	// 区段 1,  第 triloop 个三角形, 第  vertloop 个顶点, 值 v=v

            if(triloop%2==0)
                if(vertloop==0)
                {
                    line.p1x=x-0.5;
                    line.p1z=z;
                    temy=y;
                }
                else
                {
                    if(vertloop==1)
                    {
                        line.p2x=x-0.5;
                        line.p2z=z;
                    }
                    if(temy!=y)
                        iswall=true;
                }
        }
        if(iswall)
        {
            vector.append(line);
        }
    }
    //数据文件中每个三角形都以如下形式声明:
    //X1 Y1 Z1 U1 V1
    //X2 Y2 Z2 U2 V2
    //X3 Y3 Z3 U3 V3
    file.close();
}
//从数据文件中读入一个有意义的行
void MyOpenGL::readStr(QTextStream *stream, QString &string)
{
    do
    {
        string = stream->readLine();
    } while (string[0] == '/' || string[0] == '\n' || string.isEmpty());
}

void MyOpenGL::loadGLTexture()
{

    //地板贴图
    QImage image("E:/QT_file/Graphics_Project3/floor.jpeg");
    image = image.convertToFormat(QImage::Format_RGB888);
    image = image.mirrored();

    // 创建线性滤波纹理
    glBindTexture(GL_TEXTURE_2D, m_texture[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image.width(), image.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image.bits());

    //天花板贴图
    QImage image2("E:/QT_file/Graphics_Project3/floor.jpeg");
    image2 = image2.convertToFormat(QImage::Format_RGB888);
    image2 = image2.mirrored();

    glBindTexture(GL_TEXTURE_2D, m_texture[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image2.width(), image2.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image2.bits());

    //墙壁贴图
    QImage image3("E:/QT_file/Graphics_Project3/wall.jpg");
    image3 = image3.convertToFormat(QImage::Format_RGB888);
    image3 = image3.mirrored();


    glBindTexture(GL_TEXTURE_2D, m_texture[2]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image3.width(), image3.height(),
                 0, GL_RGB, GL_UNSIGNED_BYTE, image3.bits());
}

void MyOpenGL::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
    {
        renderNow();
    }
}

void MyOpenGL::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
    {
        renderNow();
    }
}
//当左右方向键按下后，旋转变量yrot。
//当前后方向键按下后，使用sine和cosine函数重新生成镜头位置。
//Piover180 折算因子用来折算度和弧度。
//walkbias是当人行走时头部产生上下摆动的幅度。
void MyOpenGL::keyPressEvent(QKeyEvent *event)
{
    int key=event->key();
    switch(key)
    {
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_F1:
        fullScreen=!fullScreen;
        if(fullScreen)
            showFullScreen();
        else
            showNormal();
        break;
    case Qt::Key_PageUp:     // 向上旋转场景
    {
        m_lookupdown-=1.0f;
        break;
    }
    case Qt::Key_PageDown:   // 向下旋转场景
    {
        m_lookupdown+=1.0f;
        break;
    }
    case Qt::Key_Right:
    {
        m_heading -=1.0f;
        m_yrot = m_heading;	 // 向左旋转场景
        break;
    }
    case Qt::Key_Left:
    {
        m_heading += 1.0f;
        m_yrot = m_heading;	// 向右侧旋转场景
        break;
    }
    case Qt::Key_Up:
    {
        m_xpos_pre=m_xpos;
        m_zpos_pre=m_zpos;
        m_xpos -= (float)sin(m_heading*piover180) * 0.1f;	// 沿游戏者所在的X平面移动
        m_zpos -= (float)cos(m_heading*piover180) * 0.1f;	// 沿游戏者所在的Z平面移动
        if(!isobstacle())
        {
            m_xpos=m_xpos_pre+(float)sin(m_heading*piover180) * 0.1f;
            m_zpos=m_zpos_pre+(float)cos(m_heading*piover180) * 0.1f;
        }
        if (m_walkbiasangle >= 359.0f)					    // 如果walkbiasangle大于359度
        {
            m_walkbiasangle = 0.0f;					        // 将walkbiasangle设为0
        }
        else
        {
            m_walkbiasangle+= 10;					        // 如果walkbiasangle < 359，则增加10
        }
        m_walkbias = (float)sin(m_walkbiasangle * piover180)/20.0f; //跳跃感
        //qDebug()<<m_xpos<<m_zpos;
        break;
    }
    case Qt::Key_Down:
    {
        m_xpos_pre=m_xpos;
        m_zpos_pre=m_zpos;
        m_xpos += (float)sin(m_heading*piover180) * 0.1f;	// 沿游戏者所在的X平面移动
        m_zpos += (float)cos(m_heading*piover180) * 0.1f;	// 沿游戏者所在的Z平面移动
        if(!isobstacle())
        {
            m_xpos=m_xpos_pre-(float)sin(m_heading*piover180) * 0.1f;
            m_zpos=m_zpos_pre-(float)cos(m_heading*piover180) * 0.1f;
        }
        if (m_walkbiasangle <= 1.0f)					    // 如果walkbiasangle小于1度
        {
            m_walkbiasangle = 359.0f;					    // 使walkbiasangle等于359
        }
        else
        {
            m_walkbiasangle-= 10;					        // 如果 walkbiasangle > 1，减去10
        }
        m_walkbias = (float)sin(m_walkbiasangle * piover180)/20.0f;	 //跳跃感
        //qDebug()<<m_xpos<<m_zpos;
        break;
    }
    }
    if(key==Qt::Key_F1||key==Qt::Key_F||key==Qt::Key_PageUp||key==Qt::Key_PageDown||key==Qt::Key_Up||key==Qt::Key_Down
            ||key==Qt::Key_Right||key==Qt::Key_Left)
    {
        renderNow();
    }
}
//碰撞检测
bool MyOpenGL::isobstacle()
{
    for(int i=0;i<vector.length();i++)
    {
        //对玩家前后移动的坐标和墙壁进行碰撞检测
        float d1,d2,d3,d4;
        d1=(m_xpos-m_xpos_pre)*(vector[i].p1z-m_zpos_pre)-(m_zpos-m_zpos_pre)*(vector[i].p1x-m_xpos_pre);
        d2=(m_xpos-m_xpos_pre)*(vector[i].p2z-m_zpos_pre)-(m_zpos-m_zpos_pre)*(vector[i].p2x-m_xpos_pre);
        d3=(vector[i].p2x-vector[i].p1x)*(m_zpos-vector[i].p1z)-(vector[i].p2z-vector[i].p1z)*(m_xpos-vector[i].p1x);
        d4=(vector[i].p2x-vector[i].p1x)*(m_zpos_pre-vector[i].p1z)-(vector[i].p2z-vector[i].p1z)*(m_xpos_pre-vector[i].p1x);
        if(d1*d2<=0&&d3*d4<=0)
        {
            //qDebug()<<vector[i].p1x<<vector[i].p1z<<vector[i].p2x<<vector[i].p2z;
           // qDebug()<<m_xpos_pre<<m_zpos_pre<<m_xpos<<m_zpos;
            return false;
        }
    }
    return true;
}
