//=============================================================================================
// Mintaprogram: Zold haromszog. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!!
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Cseh Balint Istvan
// Neptun : WRNJPE
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char * const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char * const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers

	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram; // vertex and fragment shaders
unsigned int vao;
unsigned int vbo;
const int circle_resolution = 15;
std::vector<vec3> stack;
std::vector<vec3> points;
std::vector<std::vector<vec3>> lines;
std::vector<std::vector<vec3>> circles;
std::vector<vec3> red;

void setViewPort(int view) {
    switch (view) {
        case 0:
            glViewport(0, windowHeight / 2, windowWidth / 2, windowHeight / 2);
            break;
        case 1:
            glViewport(windowWidth / 2, windowHeight / 2, windowWidth / 2, windowHeight / 2);
            break;
        case 2:
            glViewport(0, 0, windowWidth / 2, windowHeight / 2);
            break;
        case 3:
            glViewport(windowWidth / 2, 0, windowWidth / 2, windowHeight / 2);
            break;
        default:
            glViewport(0, 0, windowWidth, windowHeight);
    }
}

void drawCircle(int nVertices) {
    std::vector<vec2> vtx(nVertices);
    for (int i = 0; i < nVertices; i++) {
        float phi = i * 2.0f * M_PI / nVertices;
        vtx[i] = vec2(cosf(phi), sinf(phi)); // unit radius circle
    }
    int nBytes = vtx.size() * sizeof(vec2);
    glBufferData(GL_ARRAY_BUFFER, nBytes, vtx.data(), GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nVertices);
}

void drawHiperbola(int nVertices) {
    std::vector<vec2> vtx(nVertices+2);
    float x, y;
    for (int i = 0; i < nVertices; i++) {
        x = (float(i) / floor(float(nVertices)/2)) - 1;
        y = sqrt(1.0f + x * x) - 2;
        vtx[i] = vec2(x, y);
    }
    vtx[nVertices] = vec2(1.0f, 1.0f);
    vtx[nVertices + 1] = vec2(-1.0f, 1.0f);

    int nBytes = vtx.size() * sizeof(vec2);
    glBufferData(GL_ARRAY_BUFFER, nBytes, vtx.data(), GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nVertices+2);
}

void drawSquare(vec2 center, float size) {
    std::vector<vec2> vtx(4);
    vtx[0] = vec2(center.x-size/2, center.y - size / 2);
    vtx[1] = vec2(center.x + size / 2, center.y - size / 2);
    vtx[2] = vec2(center.x + size / 2, center.y + size / 2);
    vtx[3] = vec2(center.x - size / 2, center.y + size / 2);
    int nBytes = vtx.size() * sizeof(vec2);
    glBufferData(GL_ARRAY_BUFFER, nBytes, vtx.data(), GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

vec3 mapPoint(vec2 point, int view) {
    float x = -100, y = -100, z = -100;
    switch (view) {
        case 0:
            x = 2 * point.x / (1 - point.x * point.x - point.y * point.y);
            y = 2 * point.y / (1 - point.x * point.x - point.y * point.y);
            z = (point.x * point.x + point.y * point.y +1) / (1 - point.x * point.x - point.y * point.y);
            break;
        case 1:
            x = point.x / sqrt(1 - point.x*point.x - point.y * point.y);
            y = point.y / sqrt(1 - point.x*point.x - point.y * point.y);
            z = 1 / sqrt(1 - point.x*point.x - point.y * point.y);
            break;
        case 2:
            x = point.x;
            z = point.y+2;
            y = sqrt(-(x * x) + z * z - 1);
            break;
        case 3:
            x = point.x;
            y = point.y;
            z = sqrt(x * x + y * y + 1);
            break;
        default:
            break;
    }
    return vec3(x, y, z);
}

vec2 projectPoint(vec3 point, int view) {
    float x = -100, y = -100;
    switch (view) {
        case 0:
            x = point.x / (point.z + 1);
            y = point.y / (point.z + 1);
            break;
        case 1:
            x = point.x / point.z;
            y = point.y / point.z;
            break;
        case 2:
            if (point.y > 0) {
                x = point.x;
                y = point.z - 2;
            }
            break;
        case 3:
            x = point.x;
            y = point.y;
            break;
        default:
            break;
    }
    return vec2(x, y);
}

void drawLine(std::vector<vec3> line3, int view){
    std::vector<vec2> line2;
    for(int i=0; i < line3.size(); i++){
        if (projectPoint(line3[i],view).x != -100)
        line2.push_back(projectPoint(line3[i],view));
    }

    int nBytes = line2.size() * sizeof(vec2);
    glBufferData(GL_ARRAY_BUFFER, nBytes, line2.data(), GL_STATIC_DRAW);
    glDrawArrays(GL_LINE_STRIP, 0, line2.size());
}

void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);

    //circle
    glGenVertexArrays(1, &vao); glBindVertexArray(vao);

    glGenBuffers(1, &vbo);	glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);

    gpuProgram.create(vertexSource, fragmentSource, "outColor");
    glClearColor(0.8, 0.8, 0.8, 0); // background color | torles szinenek beallitasa
}

void drawPoincare() {
    int color_location = glGetUniformLocation(gpuProgram.getId(), "color");

    glUniform3f(color_location, 0.5f, 0.5f, 0.5f);
    setViewPort(0);
    drawCircle(circle_resolution);

    //stack
    glUniform3f(color_location, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < stack.size(); i++) {
        drawSquare(projectPoint(stack[i], 0), 0.05f);
    }

    //red
    glUniform3f(color_location, 1.0f, 0.0f, 0.0f);
    for (int i = 0; i < red.size(); i++) {
        drawSquare(projectPoint(red[i], 0), 0.05f);
    }

    //points
    glUniform3f(color_location, 1.0f, 0.0f, 0.0f);
    for (int i = 0; i < points.size(); i++) {
        drawSquare(projectPoint(points[i], 0), 0.05f);
    }

    //lines
    glUniform3f(color_location, 1.0f, 1.0f, 1.0f);
    for (int i = 0; i < lines.size(); i++) {
        drawLine(lines[i], 0);
    }
}

void drawKlein(){
    int color_location = glGetUniformLocation(gpuProgram.getId(), "color");

    glUniform3f(color_location, 0.5f, 0.5f, 0.5f);
    setViewPort(1);
    drawCircle(circle_resolution);

    //stack
    glUniform3f(color_location, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < stack.size(); i++) {
        drawSquare(projectPoint(stack[i], 1), 0.05f);
    }

    //red
    glUniform3f(color_location, 1.0f, 0.0f, 0.0f);
    for (int i = 0; i < red.size(); i++) {
        drawSquare(projectPoint(red[i], 1), 0.05f);
    }

    //points
    glUniform3f(color_location, 1.0f, 0.0f, 0.0f);
    for (int i = 0; i < points.size(); i++) {
        drawSquare(projectPoint(points[i], 1), 0.05f);
    }

    //lines
    glUniform3f(color_location, 1.0f, 1.0f, 1.0f);
    for (int i = 0; i < lines.size(); i++) {
        drawLine(lines[i], 1);
    }
}

void drawSide(){
    int color_location = glGetUniformLocation(gpuProgram.getId(), "color");

    glUniform3f(color_location, 0.5f, 0.5f, 0.5f);
    setViewPort(2);
    drawHiperbola(circle_resolution);

    //stack
    glUniform3f(color_location, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < stack.size(); i++) {
        drawSquare(projectPoint(stack[i], 2), 0.05f);
    }

    //red
    glUniform3f(color_location, 1.0f, 0.0f, 0.0f);
    for (int i = 0; i < red.size(); i++) {
        drawSquare(projectPoint(red[i], 2), 0.05f);
    }

    //points
    glUniform3f(color_location, 1.0f, 0.0f, 0.0f);
    for (int i = 0; i < points.size(); i++) {
        drawSquare(projectPoint(points[i], 2), 0.05f);
    }

    //lines
    glUniform3f(color_location, 1.0f, 1.0f, 1.0f);
    for (int i = 0; i < lines.size(); i++) {
        drawLine(lines[i], 2);
    }
}

void drawBottom() {
    int color_location = glGetUniformLocation(gpuProgram.getId(), "color");

    glUniform3f(color_location, 0.5f, 0.5f, 0.5f);
    setViewPort(3);
    drawSquare(vec2(0,0), 2);

    //stack
    glUniform3f(color_location, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < stack.size(); i++) {
        drawSquare(projectPoint(stack[i], 3), 0.05f);
    }

    //red
    glUniform3f(color_location, 1.0f, 0.0f, 0.0f);
    for (int i = 0; i < red.size(); i++) {
        drawSquare(projectPoint(red[i], 3), 0.05f);
    }

    //points
    glUniform3f(color_location, 1.0f, 0.0f, 0.0f);
    for (int i = 0; i < points.size(); i++) {
        drawSquare(projectPoint(points[i], 3), 0.05f);
    }

    //lines
    glUniform3f(color_location, 1.0f, 1.0f, 1.0f);
    for (int i = 0; i < lines.size(); i++) {
        drawLine(lines[i], 3);
    }
}

void createLine(std::vector<vec3> coord3, int nVertices){
    std::vector<vec2> coord2(2);
    coord2[0] = projectPoint(coord3[0], 1);
    coord2[1] = projectPoint(coord3[1], 1);

    vec2 v = vec2(coord2[0].x-coord2[1].x, coord2[0].y - coord2[1].y);
    vec2 n = vec2(v.y, -v.x);
    if (n.y==0) n.y+=0.1;

    std::vector<vec3> line;
    float x = 0,y = 0;
    for (float i = -1; i < 1; i+=abs(v.x/nVertices)) {
        x = i;
        y = (n.x*coord2[0].x + n.y * coord2[0].y - n.x * x) / n.y;
        if(y>=-1 and y<=1) line.push_back(mapPoint(vec2(x, y), 1));
    }
    lines.push_back(line);
}


// Window has become invalid: Redraw
void onDisplay() {
    glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer | torles

    int color_location = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(color_location, 0.5f, 0.5f, 0.5f); // 3 floats

    float MVPtransf[4][4] = { 1, 0, 0, 0,    // MVP matrix,
                              0, 1, 0, 0,    // row-major!
                              0, 0, 1, 0,
                              0, 0, 0, 1 };

    int location = glGetUniformLocation(gpuProgram.getId(), "MVP");	// Get the GPU location of uniform variable MVP
    glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);	// Load a 4x4 row-major float matrix to the specified location

    drawPoincare();

    drawKlein();

    drawSide();

    drawBottom();


    glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
    if (key == 'd') glutPostRedisplay();         // if d, invalidate display, i.e. redraw
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    // Convert to normalized device space
    /*
    float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
    float cY = 1.0f - 2.0f * pY / windowHeight;
    printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
    */
}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    if (state == GLUT_DOWN && button!=GLUT_MIDDLE_BUTTON) {
        float cX = 2.0f * pX / windowWidth - 1;
        float cY = 1.0f - 2.0f * pY / windowHeight; // flip y axis

        if (cX <= 0 && cY > 0) {
            //view 0
            cX = 2 * cX + 1;
            cY = 2 * cY - 1;
            if (cX * cX + cY * cY >= 1) return;
            stack.push_back(mapPoint(vec2(cX, cY), 0));
        }
        else if (cX > 0 && cY > 0) {
            //view 1
            cX = 2 * cX - 1;
            cY = 2 * cY - 1;
            if (cX * cX + cY * cY >= 1) return;
            stack.push_back(mapPoint(vec2(cX, cY), 1));
        }
        else if (cX <= 0 && cY <= 0) {
            //view 2
            cX = 2 * cX + 1;
            cY = 2 * cY + 1;
            if (cY <  sqrt(1.0f + cX * cX) - 2) return;
            stack.push_back(mapPoint(vec2(cX, cY), 2));
        }
        else {
            //view 3
            cX = 2 * cX - 1;
            cY = 2 * cY + 1;
            stack.push_back(mapPoint(vec2(cX, cY), 3));
        }

        if(button == GLUT_RIGHT_BUTTON){
            if(stack.size() == 1){
                points.push_back(stack[stack.size()-1]);
                red.push_back(stack[stack.size()-1]);
                stack.pop_back();
            } else if (stack.size() == 2){
                std::vector<vec3> line;
                line.push_back(stack[stack.size()-1]);
                red.push_back(stack[stack.size()-1]);
                stack.pop_back();

                line.push_back(stack[stack.size()-1]);
                red.push_back(stack[stack.size()-1]);
                stack.pop_back();

                createLine(line,circle_resolution);
            } else if (stack.size() >= 3){
                std::vector<vec3> circle;
                circle.push_back(stack[stack.size()-1]);
                red.push_back(stack[stack.size()-1]);
                stack.pop_back();

                circle.push_back(stack[stack.size()-1]);
                red.push_back(stack[stack.size()-1]);
                stack.pop_back();

                circle.push_back(stack[stack.size()-1]);
                red.push_back(stack[stack.size()-1]);
                stack.pop_back();
            }
        }

    }
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
    long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
}
