#include <string>
#include <iostream>
#include <tuple>
#include <sstream>
#include <vector>
#include <algorithm>
#include <random>

#include "../common/glincludes.h"
#include "../common/Effect.h"
#include "../common/Texture2D.h"
#include "../common/utils.h"
#include "../common/CommonDeclarations.h"
#include "../common/Cubemap.h"

#include "ShaderSources.h"
#include "Water.h"

class Scene {
public:
	Scene()
		: isLeftButtonPressed(false),
		cameraPosition(0, 0, 5), cameraRight(1, 0, 0), cameraUp(0, 1, 0), cameraLook(0, 0, -1),
		lightDirection(2.0f, 2.0f, -1.0f), sphereCenter(-0.4f, -0.75f, 0.2f), sphereRadius(0.25f),
		frame(0), distribution(-1.0f, 1.0f)
	{
		lightDirection = glm::normalize(lightDirection);
		oldCenter = sphereCenter;
		UpdateCamera();

		std::random_device rd;
		random.seed(rd());
	}

	void Init()
	{
		// Black background
		glClearColor(0, 0, 0, 1);
		
		glEnableClientState(GL_VERTEX_ATTRIB_ARRAY_POINTER);

		waterMesh.AddPlane(200, 200);
		cubeMesh.AddCube();
		cubeMesh.Triangles.erase(cubeMesh.Triangles.begin() + 4, cubeMesh.Triangles.begin() + 6);

		sideTexture.LoadFromFile("textures/cube.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);
		cubemap.LoadFromFiles("textures/sky/*.tga");
		casusticsTexture.InitializeEmpty(1024, 1024, GL_LINEAR, GL_LINEAR, GL_UNSIGNED_BYTE);

		zf::Shader waterVertex(GL_VERTEX_SHADER);
		waterVertex.CompileFile("fx/plane_vertex.glsl");

		std::string underwaterSource = std::string(shader_sources::helper) + shader_sources::waterMain + shader_sources::underwater;
		std::string aboveSource = std::string(shader_sources::helper) + shader_sources::waterMain + shader_sources::aboveWater;

		{ // water
			zf::Shader underwaterFragment(GL_FRAGMENT_SHADER), aboveFragment(GL_FRAGMENT_SHADER);
			underwaterEffect.Attach(waterVertex).Attach(underwaterFragment.CompileText(underwaterSource.c_str())).Link();
			aboveWaterEffect.Attach(waterVertex).Attach(aboveFragment.CompileText(aboveSource.c_str())).Link();
		}

		{ // sphere
			zf::Shader vertex(GL_VERTEX_SHADER), fragment(GL_FRAGMENT_SHADER);
			std::string vertexSource = std::string(shader_sources::helper) + shader_sources::sphereVertex;
			std::string fragmentSource = std::string(shader_sources::helper) + shader_sources::sphereFragment;
			sphereEffect.Attach(vertex.CompileText(vertexSource.c_str())).Attach(fragment.CompileText(fragmentSource.c_str())).Link();
		}

		{ // cube
			zf::Shader vertex(GL_VERTEX_SHADER), fragment(GL_FRAGMENT_SHADER);
			std::string vertexSource = std::string(shader_sources::helper) + shader_sources::cubeVertex;
			std::string fragmentSource = std::string(shader_sources::helper) + shader_sources::cubeFragment;
			cubeEffect.Attach(vertex.CompileText(vertexSource.c_str())).Attach(fragment.CompileText(fragmentSource.c_str())).Link();
		}

		{ // casustics
			zf::Shader vertex(GL_VERTEX_SHADER), fragment(GL_FRAGMENT_SHADER);
			std::string vertexSource = std::string(shader_sources::helper) + shader_sources::causticsVertex;
			std::string fragmentSource = std::string(shader_sources::helper) + shader_sources::causticsFragment;
			casusticsEffect.Attach(vertex.CompileText(vertexSource.c_str())).Attach(fragment.CompileText(fragmentSource.c_str())).Link();
		}

		water.Init();
	}

	void UpdateCaustics()
	{
		zf::SetRenderTo renderTo(casusticsTexture);
		water.Texture().Bind(0);

		casusticsEffect.Apply();
		glUniform3fv(casusticsEffect.Uniform("light"), 1, glm::value_ptr(lightDirection));
		glUniform1i(casusticsEffect.Uniform("water"), 0);
		glUniform3fv(casusticsEffect.Uniform("sphereCenter"), 1, glm::value_ptr(sphereCenter));
		glUniform1f(casusticsEffect.Uniform("sphereRadius"), sphereRadius);

		waterMesh.Draw();
	}

	void RenderWater(glm::mat4 &mvp)
	{
		water.Texture().Bind(0);
		sideTexture.Bind(1);
		zf::BindCubemap cubeBinding(cubemap, 2);
		casusticsTexture.Bind(3);
		zf::UseCapability c1(GL_CULL_FACE, false);

		for (int i = 0; i < 2; i++) {
			glCullFace(i ? GL_BACK : GL_FRONT);

			zf::Effect &eff = i ? underwaterEffect : aboveWaterEffect;
			eff.Apply();
			glUniform3fv(eff.Uniform("light"), 1, glm::value_ptr(lightDirection));
			glUniform1i(eff.Uniform("water"), 0);
			glUniform1i(eff.Uniform("tiles"), 1);
			glUniform1i(eff.Uniform("sky"), 2);
			glUniform1i(eff.Uniform("causticTex"), 3);
			glUniform3fv(eff.Uniform("eye"), 1, glm::value_ptr(cameraPosition));
			glUniform3fv(eff.Uniform("sphereCenter"), 1, glm::value_ptr(sphereCenter));
			glUniform1f(eff.Uniform("sphereRadius"), sphereRadius);
			glUniformMatrix4fv(eff.Uniform("MVP"), 1, false, glm::value_ptr(mvp));

			waterMesh.Draw();
		}
	}

	void RenderSphere(glm::mat4 &mvp)
	{
		water.Texture().Bind(0);
		casusticsTexture.Bind(1);

		zf::Effect &eff = sphereEffect;
		eff.Apply();
		glUniform3fv(eff.Uniform("light"), 1, glm::value_ptr(lightDirection));
		glUniform1i(eff.Uniform("water"), 0);
		glUniform1i(eff.Uniform("causticTex"), 1);
		glUniform3fv(eff.Uniform("sphereCenter"), 1, glm::value_ptr(sphereCenter));
		glUniform1f(eff.Uniform("sphereRadius"), sphereRadius);
		glUniformMatrix4fv(eff.Uniform("MVP"), 1, false, glm::value_ptr(mvp));

		// TODO: draw sphere mesh
	}

	void RenderCube(glm::mat4 &mvp)
	{
		zf::UseCapability c1(GL_CULL_FACE, true);
		water.Texture().Bind(0);
		sideTexture.Bind(1);
		casusticsTexture.Bind(2);

		zf::Effect &eff = cubeEffect;
		eff.Apply();
		glUniform3fv(eff.Uniform("light"), 1, glm::value_ptr(lightDirection));
		glUniform1i(eff.Uniform("water"), 0);
		glUniform1i(eff.Uniform("tiles"), 1);
		glUniform1i(eff.Uniform("causticTex"), 2);
		glUniform3fv(eff.Uniform("sphereCenter"), 1, glm::value_ptr(sphereCenter));
		glUniform1f(eff.Uniform("sphereRadius"), sphereRadius);
		glUniformMatrix4fv(eff.Uniform("MVP"), 1, false, glm::value_ptr(mvp));

		cubeMesh.Draw();
	}

	void Update(float seconds)
	{
		//if (seconds > 1) return;
		frame += seconds * 2;

		// Displace water around the sphere
		water.MoveSphere(oldCenter, sphereCenter, sphereRadius);
		oldCenter = sphereCenter;

		// Update the water simulation and graphics
		water.StepSimulation();
		water.StepSimulation();
		water.UpdateNormals();
		UpdateCaustics();
	}

	void Render()
	{
		// Clear the window with current clearing color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glm::mat4 model = glm::mat4();
		glm::mat4 mvp = projection * view * model;

		{ // draw scene
			zf::UseCapability c1(GL_DEPTH_TEST, true);
			RenderCube(mvp);
			RenderWater(mvp);
			//RenderSphere(mvp);
		}

		{ // draw info text
			zf::UseCapability c1(GL_DEPTH_TEST, false);
			glUseProgram(0);
			glRasterPos2f(-0.85f, 0.85f);

			std::ostringstream os;
			os << "camera position: (" << zf::ToString(cameraPosition) << ")"
			   << "\n" "camera look: (" << zf::ToString(cameraLook) << ")"
			   << "\n" "[s] -> update, [d,f,g] -> add random drop";
			std::string info = os.str();
			glutBitmapString(GLUT_BITMAP_HELVETICA_18, reinterpret_cast<const unsigned char *>(info.c_str()));
		}

		glutSwapBuffers();
	}

	void UpdateCamera()
	{
		auto position = cameraPosition;
		view = glm::lookAt(position, position + cameraLook, cameraUp);
	}

	void UpdateProjection(int viewportWidth, int viewportHeight)
	{
		float aspectRatio = 1.0f * viewportWidth / viewportHeight;
		projection = glm::perspective(45.0f, 1.0f * aspectRatio, 0.51f, 100.0f);
	}

	void OnMouseAction(int button, int state, int x, int y)
	{
		mousePosition = glm::vec2(x, y);
		if (button == GLUT_LEFT_BUTTON) {
			isLeftButtonPressed = state == GLUT_DOWN;
		} else if (button == GLUT_RIGHT_BUTTON) {
			isRightButtonPressed = state == GLUT_DOWN;
			glutPostRedisplay();
		}
	}

	void OnMouseMove(int isActive, int x, int y)
	{
		if (isActive) {
			if (isLeftButtonPressed) {
				auto newMousePosition = glm::vec2(x, y);
				auto diff = newMousePosition - mousePosition;
				mousePosition = newMousePosition;

				glm::mat4 rotation = glm::rotate(glm::mat4(), diff.y * .005f, cameraRight);
				cameraUp = (rotation * glm::vec4(cameraUp, 1)).xyz;
				cameraLook = (rotation * glm::vec4(cameraLook, 1)).xyz;
				cameraPosition = (rotation * glm::vec4(cameraPosition, 1)).xyz;

				rotation = glm::rotate(glm::mat4(), -diff.x * .005f, glm::vec3(0, 1, 0));
				cameraRight = (rotation * glm::vec4(cameraRight, 1)).xyz;
				cameraUp = (rotation * glm::vec4(cameraUp, 1)).xyz;
				cameraLook = (rotation * glm::vec4(cameraLook, 1)).xyz;
				cameraPosition = (rotation * glm::vec4(cameraPosition, 1)).xyz;

				UpdateCamera();
				glutPostRedisplay();
			} else if (isRightButtonPressed) {
				mousePosition = glm::vec2(x, y);
				glutPostRedisplay();
			}
		}
	}

	void OnMouseWheel(int button, int direction)
	{
		float value = (direction > 0 ? 1.0f : -1.0f);
		cameraPosition += cameraLook * value;
		UpdateCamera();
		glutPostRedisplay();
	}

	void OnKeyPress(unsigned char key, int x, int y)
	{
		if (key == 's') {
			Update(0.1f);
		} else if (key == 'd') {
			water.AddDrop(distribution(random), distribution(random), 0.03f, 0.03f);
		} else if (key == 'f') {
			water.AddDrop(distribution(random), distribution(random), 0.06f, 0.10f);
		} else if (key == 'g') {
			water.AddDrop(distribution(random), distribution(random), 0.10f, 0.20f);
		}
		glutPostRedisplay();
	}

	void OnSpecialKeyPress(int key, int x, int y)
	{
		glutPostRedisplay();
	}

private:
	glm::mat4 view;
	glm::mat4 projection;

	glm::vec3 cameraPosition;
	glm::vec3 cameraRight, cameraUp, cameraLook;

	bool isLeftButtonPressed;
	bool isRightButtonPressed;
	glm::vec2 mousePosition;

	glm::vec3 lightDirection;
	glm::vec3 sphereCenter, oldCenter;
	float sphereRadius;

	zf::Texture2D sideTexture, casusticsTexture;
	zf::Effect underwaterEffect, aboveWaterEffect;
	zf::Effect sphereEffect, cubeEffect, casusticsEffect;

	zf::Water water;
	zf::Cubemap cubemap;

	zf::Mesh waterMesh, sphereMesh, cubeMesh;

	float frame;
	std::mt19937 random;
	std::uniform_real_distribution<float> distribution;
};

Scene scene;

void onError(const std::string &message)
{
	std::cerr << message << std::endl;
	std::cout << "Press <Enter> to exit...";
	std::string s;
	std::getline(std::cin, s);
}

bool isRequirementsFullfiled() {
	bool isSuccess = false;
	if (!GLEW_VERSION_3_3)
		onError("Error: OpenGL 3.3 required to run this application.");
	else if (!GLEW_ARB_multitexture)
		onError("Error: missing required ARB_multitexture extension.");
	else if (!GLEW_EXT_texture_env_combine)
		onError("Error: missing required EXT_texture_env_combine extension.");
	else
		isSuccess = true;
	return isSuccess;
}

int main(int argc, char *argv[])
{
	glutInitWarningFunc([](const char *fmt, va_list ap){
		onError("GLUT warning: " + zf::vformat(fmt, ap));
	});
	glutInitErrorFunc([](const char *fmt, va_list ap){
		onError("GLUT error: " + zf::vformat(fmt, ap));
	});

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 800);
	
	glutCreateWindow("Lab 2 -- OpenGL");
	glutReshapeFunc([](int w, int h){
		glViewport(0, 0, w, h);
	});

	GLenum glewInitResult = glewInit();
	if (glewInitResult != GLEW_OK) {
		onError(std::string("GLEW error: ") + reinterpret_cast<const char *>(glewGetErrorString(glewInitResult)));
		return 1;
	}
	
	if (!isRequirementsFullfiled())
		return 2;

	try {
		scene.Init();
	} catch (zf::ShaderLoadException &ex) {
		std::string message = ex.ShaderPath().length() > 0
			? (ex.ShaderPath() + ": ") : "<in-memory shader>: ";
		onError(message + ex.what());
		return 3;
	} catch (std::runtime_error &err) {
		onError(err.what());
		return 4;
	}

	glutReshapeFunc([](int width, int height){
		glViewport(0, 0, width, height);
		scene.UpdateProjection(width, height);
	});
	glutDisplayFunc([](){
		scene.Render();
	});
	glutKeyboardFunc([](unsigned char key, int x, int y){
		scene.OnKeyPress(key, x, y);
	});
	glutSpecialFunc([](int key, int x, int y){
		scene.OnSpecialKeyPress(key, x, y);
	});
	glutMouseFunc([](int button, int state, int x, int y){
		scene.OnMouseAction(button, state, x, glutGet(GLUT_WINDOW_HEIGHT) - y);
	});
	glutMotionFunc([](int x, int y){
		scene.OnMouseMove(true, x, glutGet(GLUT_WINDOW_HEIGHT) - y);
	});
	glutPassiveMotionFunc([](int x, int y){
		scene.OnMouseMove(false, x, glutGet(GLUT_WINDOW_HEIGHT) - y);
	});
	glutMouseWheelFunc([](int button, int direction, int x, int y){
		scene.OnMouseWheel(button, direction);
	});

	glutMainLoop();
	return 0;
}
