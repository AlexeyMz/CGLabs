#include <string>
#include <iostream>
#include <tuple>
#include <sstream>

#include "../common/glincludes.h"
#include "../common/Effect.h"
#include "../common/Texture2D.h"
#include "../common/utils.h"
#include "CommonDeclarations.h"
#include "Floor.h"
#include "MirrorSphere.h"

class Scene {
public:
	Scene()
		: isLeftButtonPressed(false),
		cameraPosition(0, 0, 5), cameraRight(1, 0, 0), cameraUp(0, 1, 0), cameraLook(0, 0, -1),
		projection(glm::perspective(45.0f, 1.0f, 1.0f, 500.0f)), isBumpMappingEnabled(false)
	{
		UpdateCamera();
	}

	void Init()
	{
		// Blue background
		//glClearColor(100 / 255.f, 149 / 255.f, 237 / 255.f, 1.0f);
		glClearColor(0, 0, 0, 1);
		// Load shaders
		zf::Shader vertex(GL_VERTEX_SHADER), fragment(GL_FRAGMENT_SHADER);
		effect.Attach(vertex.CompileFile("fx/cube_vertex.glsl"))
			.Attach(fragment.CompileFile("fx/cube_fragment.glsl"))
			.Link();

		cubeTexture.LoadFromFile("textures/gateway_texture.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);
		normalSampler.LoadFromFile("textures/gateway_normals.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);

		worldUniform = effect.Uniform("Model");
		viewUniform = effect.Uniform("View");
		projectionUniform = effect.Uniform("Projection");
		lightPositionUniform = effect.Uniform("LightPosition_worldspace");
		lightColorUniform = effect.Uniform("LightColor");
		ambientUniform = effect.Uniform("Ambient");
		textureUniform = effect.Uniform("TextureUnit0");
		mv3x3Uniform = effect.Uniform("MV3x3");
		normalSamplerUniform = effect.Uniform("NormalSampler");

		floor.Init();
		mirror.Init();
		
		glEnable(GL_CULL_FACE);
		glEnableClientState(GL_VERTEX_ATTRIB_ARRAY_POINTER);
	}

	void Render()
	{
		// Clear the window with current clearing color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		effect.Apply();
		glUniformMatrix4fv(viewUniform, 1, false, glm::value_ptr(view));
		glUniformMatrix4fv(projectionUniform, 1, false, glm::value_ptr(projection));
		glUniform3fv(lightPositionUniform, 1, glm::value_ptr(cameraPosition));
		glUniform4f(lightColorUniform, 1, 1, 1, 1);
		
		// set up cube texture and normal map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture.ID());
		glUniform1i(textureUniform, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalSampler.ID());
		glUniform1i(normalSamplerUniform, 1);

		// draw objects mirror
		auto model = glm::translate(glm::scale(
			glm::translate(glm::mat4(), glm::vec3(0, floor.Height, 0)),
			glm::vec3(1, -1, 1)), glm::vec3(0, -floor.Height, 0));
		glCullFace(GL_FRONT);
		RenderObjects(model);
		glCullFace(GL_BACK);

		{ // draw floor
			zf::UseCapability c1(GL_DEPTH_TEST, false);
			zf::UseCapability c2(GL_BLEND, true);
			glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
			glBlendColor(0, 0, 0, .7f);
			floor.Draw(projection * view);
		}

		RenderObjects(glm::mat4());

		glDisable(GL_DEPTH_TEST);
		glUseProgram(0);
		glRasterPos2f(-0.85f, 0.85f);

		std::ostringstream os;
		os << "camera position: (" << zf::ToString(cameraPosition) << ")"
		   << "\n" "camera look: (" << zf::ToString(cameraLook) << ")";
		std::string info = os.str();
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, reinterpret_cast<const unsigned char *>(info.c_str()));

		glutSwapBuffers();
	}

	void RenderObjects(glm::mat4 model)
	{
		glm::vec3 p[] = {
			glm::vec3(-.5f,  .5f,  .5f),  //   1----2
			glm::vec3(-.5f,  .5f, -.5f),  //  /:   /|
			glm::vec3( .5f,  .5f, -.5f),  // 0----3 |
			glm::vec3( .5f,  .5f,  .5f),  // | 5..|.6
			glm::vec3(-.5f, -.5f,  .5f),  // |.   |/
			glm::vec3(-.5f, -.5f, -.5f),  // 4----7
			glm::vec3( .5f, -.5f, -.5f),
			glm::vec3( .5f, -.5f,  .5f),
		};
		glm::vec3 points[] = {
			p[0], p[2], p[1],  //    0 5
			p[0], p[3], p[2],  //    |/
			p[4], p[6], p[7],  // 2--o--3
			p[4], p[5], p[6],  //   /|
			p[4], p[1], p[5],  //  4 1
			p[4], p[0], p[1], 
			p[7], p[2], p[3], 
			p[7], p[6], p[2], 
			p[4], p[3], p[0], 
			p[4], p[7], p[3], 
			p[5], p[2], p[6], 
			p[5], p[1], p[2],
		};
		glm::vec3 n[] = {
			glm::vec3( 0,  1,  0),
			glm::vec3( 0, -1,  0),
			glm::vec3(-1,  0,  0),
			glm::vec3( 1,  0,  0),
			glm::vec3( 0,  0,  1),
			glm::vec3( 0,  0, -1),
		};
		glm::vec3 normals[] = {
			n[0], n[0], n[0], n[0], n[0], n[0],
			n[1], n[1], n[1], n[1], n[1], n[1],
			n[2], n[2], n[2], n[2], n[2], n[2],
			n[3], n[3], n[3], n[3], n[3], n[3],
			n[4], n[4], n[4], n[4], n[4], n[4],
			n[5], n[5], n[5], n[5], n[5], n[5],
		};
		float texCoords[] = {
			0, .75f, .5f, 1, 0, 1,
			0, .75f, .5f, .75f, .5f, 1,
			.5f, 1, 1, .75f, 1, 1,
			.5f, 1, .5f, .75f, 1, .75f,
			.5f, .5f, 0, .75f, 0, .5f,
			.5f, .5f, .5f, .75f, 0, .75f,
			.5f, .5f, 1, .75f, .5f, .75f,
			.5f, .5f, 1, .5f, 1, .75f,
			0, .25f, .5f, .5f, 0, .5f,
			0, .25f, .5f, .25f, .5f, .5f,
			1, .25f, .5f, .5f, .5f, .25f,
			1, .25f, 1, .5f, .5f, .5f,
		};
		glm::vec3 tangents[] = {
			n[3], n[3], n[3], n[3], n[3], n[3],
			n[2], n[2], n[2], n[2], n[2], n[2],
			n[4], n[4], n[4], n[4], n[4], n[4],
			n[5], n[5], n[5], n[5], n[5], n[5],
			n[3], n[3], n[3], n[3], n[3], n[3],
			n[2], n[2], n[2], n[2], n[2], n[2],
		};
		glm::vec3 bitangents[] = {
			n[5], n[5], n[5], n[5], n[5], n[5],
			n[4], n[4], n[4], n[4], n[4], n[4],
			n[0], n[0], n[0], n[0], n[0], n[0],
			n[0], n[0], n[0], n[0], n[0], n[0],
			n[0], n[0], n[0], n[0], n[0], n[0],
			n[0], n[0], n[0], n[0], n[0], n[0],
		};

		effect.Apply();
		glm::mat3 mv3x3 = glm::mat3(view * model);
		glUniformMatrix4fv(worldUniform, 1, false, glm::value_ptr(model));
		glUniformMatrix3fv(mv3x3Uniform, 1, false, glm::value_ptr(mv3x3));
		glUniform1f(ambientUniform, isBumpMappingEnabled ? .1f : 1.0f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture.ID());

		{ // draw cube
			zf::UseVertexAttribPointer position(zf::VertexAttr::POSITION);
			zf::UseVertexAttribPointer normal(zf::VertexAttr::NORMAL);
			zf::UseVertexAttribPointer texCoord0(zf::VertexAttr::TEXCOORD0);
			zf::UseVertexAttribPointer tangent(zf::VertexAttr::TANGENT);
			zf::UseVertexAttribPointer bitangent(zf::VertexAttr::BITANGENT);
			glVertexAttribPointer(position, 3, GL_FLOAT, false, 0, points);
			glVertexAttribPointer(normal, 3, GL_FLOAT, false, 0, normals);
			glVertexAttribPointer(texCoord0, 2, GL_FLOAT, false, 0, texCoords);
			glVertexAttribPointer(tangent, 3, GL_FLOAT, false, 0, tangents);
			glVertexAttribPointer(bitangent, 3, GL_FLOAT, false, 0, bitangents);
			glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
		}

		mirror.Draw(
			model * glm::translate(glm::mat4(), glm::vec3(2, 3, 2)), view, projection,
			floor.Texture, 1.0f / 100, floor.Height);
	}

	void UpdateCamera()
	{
		auto position = cameraPosition;
		position.y = std::max(0.0f, position.y);
		view = glm::lookAt(position, position + cameraLook, cameraUp);
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
		glutPostRedisplay();
	}

	void OnSpecialKeyPress(int key, int x, int y)
	{
		//const float MovementVelocity = 0.05f;
		//if (key == GLUT_KEY_LEFT) {
		//	cameraPosition -= cameraRight * MovementVelocity;
		//	UpdateCamera();
		//} else if (key == GLUT_KEY_RIGHT) {
		//	cameraPosition += cameraRight * MovementVelocity;
		//	UpdateCamera();
		//} else if (key == GLUT_KEY_UP) {
		//	cameraPosition += cameraLook * MovementVelocity;
		//	UpdateCamera();
		//} else if (key == GLUT_KEY_DOWN) {
		//	cameraPosition -= cameraLook * MovementVelocity;
		//	UpdateCamera();
		//}

		glutPostRedisplay();
	}

	void ToggleIsBumpEnabled()
	{
		isBumpMappingEnabled = !isBumpMappingEnabled;
		glutPostRedisplay();
	}

private:
	zf::Effect effect;
	zf::Texture2D cubeTexture, normalSampler;
	GLint worldUniform, viewUniform, projectionUniform, mv3x3Uniform;
	GLint lightPositionUniform, lightColorUniform, ambientUniform;
	GLint textureUniform, normalSamplerUniform;

	glm::mat4 view;
	glm::mat4 projection;

	glm::vec3 cameraPosition;
	glm::vec3 cameraRight, cameraUp, cameraLook;

	zf::Floor floor;
	zf::MirrorSphere mirror;

	bool isLeftButtonPressed;
	bool isRightButtonPressed;
	glm::vec2 mousePosition;

	bool isBumpMappingEnabled;
};

Scene scene;

void onError(std::string message)
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
		onError(ex.what());
		return 3;
	} catch (std::runtime_error &err) {
		onError(err.what());
		return 4;
	}

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

	int menu = glutCreateMenu([](int selectedItem){
		if (selectedItem == 1)
			scene.ToggleIsBumpEnabled();
	});
	glutAddMenuEntry("Toggle Bump Mapping", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutMainLoop();
	return 0;
}
