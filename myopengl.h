#ifndef MYOPENGL_H
#define MYOPENGL_H

#include <QWindow>
#include <QOpenGLFunctions_1_1>
#include <QKeyEvent>
#include <QTextStream>
#include <QPoint>
#include <QVector>
#include <algorithm>


typedef struct tagVERTEX	// 创建Vertex顶点结构
{
    float x, y, z;			// 3D 坐标
    float u, v;				// 纹理坐标
} VERTEX;					// 命名为VERTEX

typedef struct tagTRIANGLE	// 创建Triangle三角形结构
{
    VERTEX vertex[3];	    // VERTEX矢量数组，大小为3
}TRIANGLE;                  // 命名为 TRIANGLE

typedef struct tagSECTOR	// 创建Sector区段结构
{
    int numtriangles;		// Sector中的三角形个数
    TRIANGLE* triangle;		// 指向三角数组的指针
} SECTOR;					// 命名为SECTOR
//隔断线
typedef struct tagLINE
{
    float p1x,p1z,p2x,p2z;
}LINE;

const float piover180 = 0.0174532925f;

class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;

class MyOpenGL : public QWindow, QOpenGLFunctions_1_1
{
    Q_OBJECT
public:
    explicit MyOpenGL(QWindow *parent = 0);
    ~MyOpenGL();

    virtual void render(QPainter *painter);
    virtual void render();
    virtual void initialize();

public slots:
    void renderNow();

protected:
    void exposeEvent(QExposeEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event); // 键盘事件

private:
    void setupWorld();//构建世界
    void readStr(QTextStream *stream, QString &string);//读取数据
    void loadGLTexture();//加载纹理
    bool isobstacle();//碰撞检测

private:
    QOpenGLContext *m_context;

    SECTOR m_sector1;

    GLfloat m_yrot;//玩家俯仰角
    GLfloat m_xpos;//玩家坐标
    GLfloat m_zpos;
    GLfloat m_xpos_pre;//玩家移动前坐标
    GLfloat m_zpos_pre;
    GLfloat m_heading;//玩家横向角度
    GLfloat m_walkbias;//跳跃感
    GLfloat m_walkbiasangle;
    GLfloat m_lookupdown;

    GLuint	m_filter;
    GLuint	m_texture[3];
    bool fullScreen;
    QVector<LINE> vector;//墙壁
};

#endif // MYOPENGL_H
