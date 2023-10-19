//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
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
std::vector<vec3> userPoints;

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
    float x, y, z = -100;
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
    float x, y = -100;
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

void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);

    //circle
    glGenVertexArrays(1, &vao); glBindVertexArray(vao);

    glGenBuffers(1, &vbo);	glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);

    gpuProgram.create(vertexSource, fragmentSource, "outColor");
    glClearColor(0.8, 0.8, 0.8, 0); // background color | törlés színének beállítása
}


// Window has become invalid: Redraw
void onDisplay() {
    glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer | törlés

    int color_location = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(color_location, 0.5f, 0.5f, 0.5f); // 3 floats

    float MVPtransf[4][4] = { 1, 0, 0, 0,    // MVP matrix, | ignoráljuk
                              0, 1, 0, 0,    // row-major!
                              0, 0, 1, 0,
                              0, 0, 0, 1 };

    int location = glGetUniformLocation(gpuProgram.getId(), "MVP");	// Get the GPU location of uniform variable MVP
    glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);	// Load a 4x4 row-major float matrix to the specified location

    //Poincaré
    glUniform3f(color_location, 0.5f, 0.5f, 0.5f);
    setViewPort(0);
    drawCircle(circle_resolution);

    glUniform3f(color_location, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < userPoints.size(); i++) {
        drawSquare(projectPoint(userPoints[i], 0), 0.05f);
    }

    //Klein
    glUniform3f(color_location, 0.5f, 0.5f, 0.5f);
    setViewPort(1);
    drawCircle(circle_resolution);

    glUniform3f(color_location, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < userPoints.size(); i++) {
        drawSquare(projectPoint(userPoints[i], 1), 0.05f);
    }

    //Oldal
    glUniform3f(color_location, 0.5f, 0.5f, 0.5f);
    setViewPort(2);
    drawHiperbola(circle_resolution);

    glUniform3f(color_location, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < userPoints.size(); i++) {
        drawSquare(projectPoint(userPoints[i], 2), 0.05f);
        printf("%f, %f \n", projectPoint(userPoints[i], 2).x, projectPoint(userPoints[i], 2).y);
    }

    //Alul
    glUniform3f(color_location, 0.5f, 0.5f, 0.5f);
    setViewPort(3);
    drawSquare(vec2(0,0), 2);

    glUniform3f(color_location, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < userPoints.size(); i++) {
        drawSquare(projectPoint(userPoints[i], 3), 0.05f);
    }


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
    if (state == GLUT_DOWN) {
        float cX = 2.0f * pX / windowWidth - 1;
        float cY = 1.0f - 2.0f * pY / windowHeight; // flip y axis

        if (cX <= 0 && cY > 0) {
            //view 0
            cX = 2 * cX + 1;
            cY = 2 * cY - 1;
            userPoints.push_back(mapPoint(vec2(cX, cY), 0));
        }
        else if (cX > 0 && cY > 0) {
            //view 1
            cX = 2 * cX - 1;
            cY = 2 * cY - 1;
            userPoints.push_back(mapPoint(vec2(cX, cY), 1));
        }
        else if (cX <= 0 && cY <= 0) {
            //view 2
            cX = 2 * cX + 1;
            cY = 2 * cY + 1;
            userPoints.push_back(mapPoint(vec2(cX, cY), 2));
        }
        else {
            //view 3
            cX = 2 * cX - 1;
            cY = 2 * cY + 1;
            userPoints.push_back(mapPoint(vec2(cX, cY), 3));
        }
    }

    /*
    char * buttonStat;
    switch (state) {
    case GLUT_DOWN: buttonStat = "pressed"; break;
    case GLUT_UP:   buttonStat = "released"; break;
    }

    switch (button) {
    case GLUT_LEFT_BUTTON:   printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);   break;
    case GLUT_MIDDLE_BUTTON: printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY); break;
    case GLUT_RIGHT_BUTTON:  printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);  break;
    }
    */
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
    long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
}
