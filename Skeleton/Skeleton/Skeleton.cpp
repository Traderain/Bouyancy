//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2018. osztol.
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
// Nev    : Hambalkó Bence
// Neptun : SRGTM8
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
		outColor = vec4(1, 0, 0, 1);	// computed color is the color of the primitive
	}
)";

// check if shader could be compiled
void checkShader(unsigned int shader, char* message) {
	int OK;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &OK);
	if (!OK) { printf("%s!\n", message); }
}

// check if shader could be linked
void checkLinking(unsigned int program) {
	int OK;
	glGetProgramiv(program, GL_LINK_STATUS, &OK);
	if (!OK) { printf("Failed to link shader program!\n");}
}

GPUProgram gpuProgram; // vertex and fragment shaders
unsigned int vao;	   // virtual world on the GPU

// handle of the shader program
unsigned int shaderProgram;

struct Fmat4 {
	float m[4][4];
public:
	Fmat4() {}
	Fmat4(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33) {
		m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
		m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
		m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
		m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
	}

	Fmat4 operator*(const Fmat4& right) const {
		Fmat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = 0;
				for (int k = 0; k < 4; k++) result.m[i][j] += m[i][k] * right.m[k][j];
			}
		}
		return result;
	}

	Fmat4 resize(const float fl) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				m[i][j] = fl * m[i][j];
			}
		}
		return *this;
	}

	operator float* () { return &m[0][0]; }
};

class Ball
{
	unsigned int vaoCircle;
	int circleRes = 20;
	const float toRadian = 2 * M_PI / circleRes;
	const float ellipseX = 0.1;
	const float ellipseY = 0.1;
	float dt = 0;
public:
	Ball() {}
	float x_initial;
	float y_initial;
	float x;
	float y;
	

	void Create()
	{
		glGenVertexArrays(1, &vaoCircle);
		glBindVertexArray(vaoCircle);

		unsigned int vbo;

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		float* vertexCoords = new float[circleRes * 2];
		for (int i = 0; i < circleRes; i++)
		{
			vertexCoords[2 * i] = cos(i * toRadian) * ellipseX;
			vertexCoords[2 * i + 1] = sin(i * toRadian) * ellipseY;
		}

		glBufferData(GL_ARRAY_BUFFER,
			sizeof(float) * circleRes * 2,
			vertexCoords,
			GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);

		glVertexAttribPointer(0,
			2, GL_FLOAT,
			GL_FALSE,
			0, NULL);
	}

	void Draw()
	{
		Fmat4 locationMatrix(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			x, y, 0, 1);
		int location = glGetUniformLocation(shaderProgram, "MVP");
		if (location >= 0)
			glUniformMatrix4fv(location, 1, GL_TRUE, locationMatrix);
		else
			printf("uniform MVP cannot be set\n");

		location = glGetUniformLocation(shaderProgram, "color");
		if (location >= 0) glUniform3f(location, 0, 1.0f, 0);
		else printf("uniform color cannot be set\n");

		location = glGetUniformLocation(shaderProgram, "hasTexture");
		if (location >= 0) glUniform1i(location, 0);
		else printf("uniform texture cannot be set\n");

		glBindVertexArray(vaoCircle);
		glDrawArrays(GL_TRIANGLE_FAN, 0, circleRes);


	}

	void Animate(float t)
	{
		float grav = -1;
		y += grav * (t-dt);
		dt = t;
	}

	void handleMouse()
	{
	}

	void handleMotion(float t)
	{

	}


};

Ball ball;


// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);

	ball.Create();

	// Create vertex shader from string
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	if (!vertexShader) { printf("Error in vertex shader creation\n"); exit(1); }
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	checkShader(vertexShader, "Vertex shader error");

	// Create fragment shader from string
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (!fragmentShader) { printf("Error in fragment shader creation\n"); exit(1); }
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	checkShader(fragmentShader, "Fragment shader error");

	// Attach shaders to a single program
	shaderProgram = glCreateProgram();
	if (!shaderProgram) { printf("Error in shader program creation\n"); exit(1); }
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	// Connect the fragmentColor to the frame buffer memory
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");	// fragmentColor goes to the frame buffer memory

																// program packaging
	glLinkProgram(shaderProgram);
	checkLinking(shaderProgram);
	// make this program run
	glUseProgram(shaderProgram);
}



// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0, 0, 0, 0);     // background color
	glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer
	
	ball.Draw();
	

	glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	if (key == 'r') 
		glutPostRedisplay();         // if d, invalidate display, i.e. redraw

	if (key == 'a')
		ball.x += 1 / 10;
	if (key == 'd')
		ball.x -= 1 / 10;
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
	float cY = 1.0f - 2.0f * pY / windowHeight;
	printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
	float cY = 1.0f - 2.0f * pY / windowHeight;

	ball.x = cX;
	ball.x_initial = cX;
	ball.y = cY;
	ball.y_initial = cY;

	char * buttonStat;
	switch (state) {
		case GLUT_DOWN: 
		{
			buttonStat = "pressed"; 
			break;
		}
		case GLUT_UP:
		{
			buttonStat = "released"; 
			break;
		}
	}

	switch (button) {
		case GLUT_LEFT_BUTTON:   
		{
			printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);
			break;
		}
		case GLUT_MIDDLE_BUTTON:
		{
			printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);
			break;
		}
		case GLUT_RIGHT_BUTTON: 
		{
			printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);
			break;
		}
	}
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
	float sec = time / 1000.0f;
	ball.Animate(sec);
	glutPostRedisplay();
}
