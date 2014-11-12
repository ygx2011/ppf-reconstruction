//
//  Visualize.cpp
//  PointPairFeatures
//
//  Created by Adrian Haarbach on 29.06.14.
//  Copyright (c) 2014 Adrian Haarbach. All rights reserved.
//
#include "Visualize.h"
#include <iostream>

using namespace std;


Visualize* Visualize::instance = 0;

Visualize* Visualize::getInstance(){
    if(instance == 0){
        instance = new Visualize();
    }
    return instance;
}

Visualize::Visualize()
    : m_Scene(true)
    , m_Highlight(false)
    , m_ColorMaterial(true)
    , m_Estimate(true)
    , m_Ambient(true)
    , m_Origin(true)
    , m_Normals(false)
    , m_Buckets(false)
    , m_Voxels(false)
    , modifier(-1)
    , angle(0.0)
    , angle2(0.0)
    , angle3(0.0)
    , mouseButton(0)
    , zoom(15)
    , offsetY(-0.075f)
    , offsetX(0)
    , current_object(0)
    , m_Model(true)
{
    std::cout<<"Visualize construct"<<std::endl;
}

//draws cylinder towards z direction
void Visualize::drawCylinderAdvanced(double r, double l, bool coverback, bool coverfront, bool normalInwards){
    int i;
	int n = 20;
    
	int z=normalInwards ? -1 : 1;
    
    for(i=0;i<2*n;i++){
        glBegin(GL_POLYGON);
        // Explanation: the normal of the whole polygon is the coordinate of the center of the polygon for a sphere
        glNormal3d(sin((i+0.5)*M_PI/n)*z,cos((i+0.5)*M_PI/n)*z,0); //middle
        glVertex3d(r*sin(i*M_PI/n),r*cos(i*M_PI/n),0);
        glVertex3d(r*sin((i+1)*M_PI/n),r*cos((i+1)*M_PI/n),0);
        glVertex3d(r*sin((i+1)*M_PI/n),r*cos((i+1)*M_PI/n),l);
        glVertex3d(r*sin(i*M_PI/n),r*cos(i*M_PI/n),l);
        glEnd();
        //bottom
        if(coverback){
            glBegin(GL_POLYGON);
            glNormal3d(0,0,-1*z); //middle
            glVertex3d(r*sin(i*M_PI/n),r*cos(i*M_PI/n),0);
            glVertex3d(r*sin((i+1)*M_PI/n),r*cos((i+1)*M_PI/n),0);
            glVertex3d(0,0,0);
            glEnd();
        }
        //top
        if(coverfront){
            glBegin(GL_POLYGON);
            glNormal3d(0,0,1*z); //middle
            glVertex3d(r*sin(i*M_PI/n),r*cos(i*M_PI/n),l);
            glVertex3d(r*sin((i+1)*M_PI/n),r*cos((i+1)*M_PI/n),l);
            glVertex3d(0,0,l);
            glEnd();
        }
    }
    
}

void Visualize::drawCylinder(double r, double l)
{
	drawCylinderAdvanced(r,l,true,true,false);
}

// for debugging, press o to display

void Visualize::drawOrigin(){
	glPushMatrix();
	glColor3f(0,0,1);
	drawCylinder(0.02,1); //z blue
	glRotatef(90,0,1,0);
	glColor3f(1,0,0);
	drawCylinder(0.02,1); //x  red
	glRotatef(-90,1,0,0);
	glColor3f(0,1,0);
	drawCylinder(0.02,1); //y  green
	glPopMatrix();
}

void Visualize::drawNormals(MatrixXf m, RowVector3f color){
    glLineWidth(0.02f);
    glBegin(GL_LINES);
    glColor3f(color(0),color(1),color(2));
    for (int i=0; i<m.rows(); i++) {
        RowVector3f p=m.block(i, 0, 1, 3);
        RowVector3f n=m.block(i, 3, 1, 3);
        
        n.normalize();
        n/=100;
        RowVector3f q=p+n;
        
        glVertex3d(p(0),p(1),p(2));
        glVertex3d(q(0),q(1),q(2));
    }
	glEnd();
    
}

//draws a square r/2 in front of x y plane with normal facing towards viewer;
void Visualize::drawPointCloud(MatrixXf m, RowVector3f color, float pointSize){
    glPointSize(pointSize);
    glColor3f(color(0),color(1),color(2));
    glBegin(GL_POINTS);
    for (int i=0; i<m.rows(); i++) {
        glVertex3f(m(i,0),m(i,1),m(i,2));
    }
	glEnd();
}

void Visualize::drawAll(MatrixXf m, RowVector3f color, RowVector3f colorNormals){
    drawPointCloud(m, color);
    
    if(m_Normals) drawNormals(m,colorNormals);
    if(m_Buckets) drawPPfs(b[bucketIndex].second, m);
    if(m_Voxels)  drawCubes(m,ddist);
}


void Visualize::drawPPF(int i, int j, MatrixXf m)
{
    glLineWidth(0.05f);
    glBegin(GL_LINE_STRIP);
        RowVector3f p=m.block(i, 0, 1, 3);
        RowVector3f n=m.block(i, 3, 1, 3);
        RowVector3f p2=m.block(j, 0, 1, 3);
        RowVector3f n2=m.block(j, 3, 1, 3);
        n.normalize();
        n/=100;
        n2.normalize();
        n2/=100;
        RowVector3f q=p+n;
        RowVector3f q2=p2+n2;
    
        glColor3f(0.0,0.2,1);
        glVertex3d(q(0),q(1),q(2)); //first normal
        glVertex3d(p(0),p(1),p(2));
    
        glColor3f(0.8,0,0.8);
        glVertex3d(p2(0),p2(1),p2(2));  //distance line

        glColor3f(0.0,0.2,1);
        glVertex3d(q2(0),q2(1),q2(2));  //second normal
    
	glEnd();
}

void Visualize::drawLines(vector<int> vertices)
{
    glLineWidth(0.05f);

    glBegin(GL_LINES);
    glColor3f(0.8,0,0.8);

    int n = vertices.size();

    for (int i = 0; i < n; ++i) {
        RowVector3f p1=scene.block(i, 0, 1, 3);
        RowVector3f p2=modelT.block(vertices[i], 0, 1, 3);

        glVertex3f(p1(0),p1(1),p1(2));  //distance line
        glVertex3f(p2(0),p2(1),p2(2));  //distance line
    }

    glEnd();
}

void Visualize::drawLines(vector<Vector3f> v1, vector<Vector3f> v2)
{
    glLineWidth(0.05f);

    glBegin(GL_LINES);
    glColor3f(0.8,0,0.8);

    int n = v1.size();

    for (int i = 0; i < n; ++i) {
        Vector3f p1=v1[i];
        Vector3f p2=v2[i];

        glVertex3f(p1(0),p1(1),p1(2));  //distance line
        glVertex3f(p2(0),p2(1),p2(2));  //distance line
    }

    glEnd();
}

void Visualize::drawPPfs(Bucket b,MatrixXf m){
    for (auto it : b){
        drawPPF(it.i, it.j,m);
    }
}

void Visualize::drawCubes(MatrixXf C, double size){
    glColor3f(0.9,0.9,0.9);
    for (int i=0; i<C.rows(); i++) {
        glPushMatrix();
        glTranslated(C(i,0), C(i,1), C(i,2));
        glutWireCube(size);
        glPopMatrix();
    }
}

void Visualize::drawMatches(Matches matches){
    Match match=matches[bucketIndex];
    drawPPfs(match.modelPPFs, model);
    drawPPF(match.scenePPF.i, match.scenePPF.j, scene);
}

void Visualize::printMatches(Matches matches){
    Match match=matches[bucketIndex];
    
    cout<<"scene:";
    match.scenePPF.print();
    cout<<"model:";
    match.modelPPFs[0].print();
    /*for (auto it : match.modelPPFs) {
        it.ppf.print();
    }*/
    cout<<endl;
}

double Visualize::getRotationAngleApprox(double xdiff, double ydiff, double x, double y){
	int xs=x>0 ? 1 : -1;
	int ys=y>0 ? 1 : -1;
	std::cout<<x<<"+"<<xdiff<<" "<<y<<"+"<<ydiff<<std::endl;
	return ydiff*xs+xdiff*ys;
}


void Visualize::display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
    glTranslatef(0, 0, -6);
    
    glRotatef(angle3, 0.0, 0.0, 1.0);
    glRotatef(angle2, 1.0, 0.0, 0.0);
    glRotatef(angle, 0.0, 1.0, 0.0);
    
    glScalef(zoom,zoom,zoom);
    
    glTranslatef(offsetX, offsetY, 0);
        
    if(m_Origin) drawOrigin();
    
    //glRED;
    //drawPointCloud(ms.at(current_object).first,ms.at(current_object).second);
    
    switch (current_object) {
		case 0:
            for(auto it : ms){
                MatrixXf m = it.first;
                RowVector3f color = RowVector3f(0,1,0); //it.second
                drawPointCloud(m,color,10.0f);
            }
            if(m_Model) drawAll(model,RowVector3f(1,0,0),RowVector3f(0,1,0.2)); //red green
            if(m_Scene) drawAll(scene,RowVector3f(1,0.5,0),RowVector3f(0,0.5,1)); //orange blue

            if(m_Estimate && modelT.rows()>0) drawPointCloud(modelT, RowVector3f(1,0,1),8.0f);//magenta

            if(matches.size()>0) drawMatches(matches);
            if(closestPtsSceneToModel.size()>0) drawLines(closestPtsSceneToModel);
            if(src.size()>0) drawLines(src,dst);

			break;
		default:
			break;
    };
	glPopMatrix();
	glutSwapBuffers ();
}

void Visualize::displayW(void){
    instance->display();
}

void Visualize::bucketInfo(){
    printMatches(matches);

    //Match match=matches[bucketIndex];
    //cout<<"bucketIndex= "<<bucketIndex<<"i="<<match.scenePPF.i<<"j="<<match.scenePPF.j<<"modelSize"<<match.modelPPFs.size()<<endl;
    
    
    
    //" key="<<b[bucketIndex].first<<" size="<<b[bucketIndex].second.size()<<endl;
}


void Visualize::keyboard (unsigned char key, int x, int y)
{
    //std::cout<<"keyboard: "<<key<< " at " << x << y <<std::endl;
    lastKey=key;

	switch (key) {
        case 'p':
        case 'P':
            printMatches(matches);
            break;
        case 'w':
        case 'W':
            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
            break;
//        case 'v':
//        case 'V':
//            glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
//            break;
        case 's':
        case 'S':
            m_Scene = !m_Scene;
            break;
        case 'h':
        case 'H':
            m_Highlight = !m_Highlight;
            break;
        case 'c':
        case 'C':
            m_ColorMaterial = !m_ColorMaterial;
            break;
        case 'e':
        case 'E':
            m_Estimate = !m_Estimate;
            break;
        case 'a':
        case 'A':
            m_Ambient = !m_Ambient;
            break;
        case 'o':
        case 'O':
            m_Origin = !m_Origin;
            break;
        case 'n':
        case 'N':
            m_Normals = !m_Normals;
            break;
        case 'm':
        case 'M':
            m_Model = !m_Model;
            break;
        case 'b':
        case 'B':
            m_Buckets = !m_Buckets;
            break;
        case 'v':
        case 'V':
            m_Voxels = !m_Voxels;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            current_object = key - '1';
            //m=ms.at(current_object).first;
            break;
            
        case 'Q':
            glutLeaveMainLoop();
            exit(0);
            break;
        case 'q':
            break;
        case '+':
            bucketIndex++;
            bucketInfo();
            break;
        case '-':
            if(bucketIndex>=1) bucketIndex--;
            bucketInfo();
            break;
            
        default:
            break;
	}
    
	glutPostRedisplay();
}

void Visualize::keyboardW (unsigned char key, int x, int y){
    instance->keyboard(key, x, y);
}

void Visualize::mouse(int button, int state, int x, int y)
{
    //printf("button=%d %s At %d %d\n", button, (state == GLUT_DOWN) ? "Down" : "Up", x, y);
    
    modifier=glutGetModifiers();
    
    if (state == GLUT_DOWN) {
        mouseButton = button;
        moving = 1;
        startx = x;
        starty = y;
    }
    if (state == GLUT_UP) {
        mouseButton = button;
        moving = 0;
    }
}

void Visualize::mouseW(int button, int state, int x, int y){
    instance->mouse(button, state, x, y);
}

void Visualize::motion(int x, int y)
{
    if (moving) {
        if(mouseButton==GLUT_LEFT_BUTTON)
        {
            if (modifier==GLUT_ACTIVE_CTRL) {
                offsetY-=(y-starty)*0.001;
                offsetX+=(x-startx)*0.001;
            }else{
                angle = angle + (x - startx);
                angle2 = angle2 + (y - starty);
            }
        }else if(mouseButton==GLUT_MIDDLE_BUTTON || (mouseButton==GLUT_RIGHT_BUTTON && modifier==GLUT_ACTIVE_ALT)){ //if we dont have mousewheel
        	int xCenter=x-(WINDOW_SIZE/2);
        	int yCenter=y-(WINDOW_SIZE/2);
        	int startxCenter=startx-(WINDOW_SIZE/2);
        	int startyCenter=starty-(WINDOW_SIZE/2);
        	angle3 -=(atan2(yCenter,xCenter)-atan2(startyCenter,startxCenter))*(180/M_PI); //rotate object around z axis with the angle corresponding to polar coordinates of mouse displacement
            //angle3 -= getRotationAngleApprox(xCenter-startxCenter,startyCenter-yCenter,xCenter,yCenter);
        }else if(mouseButton==GLUT_RIGHT_BUTTON){
        	zoom += (((y-starty)+(x-startx))*0.1); //allows to mirror the object if zoom <0
        }
        startx = x;
        starty = y;
        glutPostRedisplay();
    }
    
}

void Visualize::motionW(int x, int y){
    instance->motion(x, y);
}

void Visualize::idleW(){
    if(getInstance()->doUpdate){
        glutPostRedisplay();
        getInstance()->doUpdate=false;
    }
}


int Visualize::mainVisualize(int argc, char **argv)
{
    
    //instance = this;
    
	cout<<"Adrian's Point Cloud Visualizer"<< endl<< endl;
    
	cout << "1-9: Draw different objects"<<endl;
	cout << "Q: Quit" <<endl;
    
	cout << "Left mouse click and drag: rotate the object"<<endl;
	cout << "Right mouse click and drag: zooming"<<endl;
    cout << "Control + Left mouse click and drag: zooming"<<endl;
        
    
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize (WINDOW_SIZE, WINDOW_SIZE);
	glutInitWindowPosition (50, 50);
	glutCreateWindow ("Adrian's Point Cloud Visualizer");
	glClearColor (1.0,1.0,1.0, 1.0);

    /*
    CGLError err;
    CGLContextObj ctx = CGLGetCurrentContext();

    // Enable the multi-threading
    err =  CGLEnable( ctx, kCGLCEMPEngine);

    if (err != kCGLNoError )
    {
        cout<<"no multithread available"<<endl;
         // Multi-threaded execution is possibly not available
         // Insert your code to take appropriate action
    } */
    
    
    // store a call to a member function
//    std::function<void(const Foo&, int)> f_add_display = &Visualize::display;
//    const Foo foo(314159);
//    f_add_display(foo, 1);
    
    
    //auto ddisp = std::mem_fn(&Visualize::display);
	glutDisplayFunc(displayW);
	glutMouseFunc(mouseW);
	glutMotionFunc(motionW);
	glutKeyboardFunc(keyboardW);

    //glutTimerFunc();
   // glutIdleFunc(idleW);
	//setupLighting();
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);
    
    glMatrixMode(GL_PROJECTION);
    gluPerspective( /* field of view in degree */ 40.0,
                   /* aspect ratio */ 1.0,
                   /* Z near */ 1.0, /* Z far */ 80.0);
	glMatrixMode(GL_MODELVIEW);
    
    
    
    //glutMainLoop();
    
	return 0;
}


void Visualize::visualize(){
    getInstance()->mainVisualize(0,0);
}

void Visualize::start(){
    cout<<"start Visualize thread "<<endl;
    getInstance()->visThread = new std::thread(visualize);
}

void Visualize::waitKeyQuit(){
    getInstance()->visThread->join();
}

void Visualize::spin(){
    while(Visualize::waitKey('q')){
        glutPostRedisplay();
        glutMainLoopEvent();
    }
}

void Visualize::spin(int i){
    while(i-- > 0){
        std::chrono::milliseconds dura( 10 );
        std::this_thread::sleep_for( dura );
        glutPostRedisplay();
        glutMainLoopEvent();
    }
}

bool Visualize::waitKeyInst(unsigned char key){
    //cout<<"waiting for "<<key<<endl;
    std::chrono::milliseconds dura( 10 );
    //while(true){
        std::this_thread::sleep_for( dura );
        //sleep(1);
        if(lastKey==key){
            lastKey=-1;
            cout<<"ok, exit waiting = "<<endl;
            return false;
        }else{
            return true;
        }
    //}
}

bool Visualize::waitKey(unsigned char key){
    return getInstance()->waitKeyInst(key);
}

void Visualize::update(){
    cout<<"Visualize::update"<<endl;
    getInstance()->doUpdate=true;
}
