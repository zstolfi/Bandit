#pragma once
#include "util.hh"
#include <glad/glad.h> // Must be included before GLFW.
#include <GLFW/glfw3.h>

// Programs right now can only have one monolithic window. When this window
// closes, everything closes.
class Window {
	GLFWwindow* m_handler {};
	GLuint m_shaderProgram {};
	GLuint m_VBO {}, m_VAO {}, m_EBO {};

public:
	Window(
		unsigned width, unsigned height, std::string title,
		void (* renderLoop)(Window&)
	) {
		setupErrorLog();
		setupWindow(width, height, title);
		setupScene();
		while (!glfwWindowShouldClose(m_handler)) {
			renderLoop(*this);
		}
	}

	~Window() {
		glDeleteBuffers(1, &m_VBO);
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteProgram(m_shaderProgram);
		glfwTerminate();
	}

	// Getters
	GLFWwindow* handler() const { return m_handler; }
	GLuint shaderProgram() const { return m_shaderProgram; };
	GLuint VBO() const { return m_VBO; };
	GLuint VAO() const { return m_VAO; };

private:
	void setupErrorLog() {
		glfwSetErrorCallback([] (int code, char const* message) {
			std::print(stderr, "GLFW error {}: {}\n", code, message);
		});
	}

	void setupWindow(unsigned width, unsigned height, std::string title) {
		// Initialize with the correct OpenGL versioning.
		if (!glfwInit()) exit("glfwInit: Unable to start GLFW.\n");
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#		if __APPLE__
 			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#		endif

		// Create window.
		m_handler = glfwCreateWindow(width, height, title.c_str(), {}, {});
		if (!m_handler) exit("glfwCreateWindow: Unable to create window.\n");
		glfwMakeContextCurrent(m_handler);

		// GLAD is used to make calling GL functions easier.
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			exit("gladLoadGLLoader: Unable to start GLAD.\n");
		}

		// Set our width and height, and respond when a user resizes.
		glViewport(0, 0, width, height);
		glfwSetFramebufferSizeCallback(
			m_handler,
			[] (GLFWwindow* /**/, int newWidth, int newHeight) {
				glViewport(0, 0, newWidth, newHeight);
			}
		);
	}

	// TODO Separate these to their own .glsl files.
	static char constexpr const* vertexShaderSource {R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;

		void main() {
			gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
		}
	)"};

	static char constexpr const* fragmentShaderSource {R"(
		#version 330 core
		out vec4 FragColor;

		void main() {
			FragColor = vec4(1.0, 0.5, 0.2, 1.0);
		}
	)"};

	enum ObjectType {
		Shader, Program
	};

	template <ObjectType OT>
	void checkObject(GLuint object, std::string name) {
		auto [getObject, getObjectLog, status] = (OT == Shader)
		?	std::tuple {glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS}
		:	std::tuple {glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS};
		auto action = (OT == Shader)? "compile": "link";

		GLint successful {}, logLength {};
		getObject(object, status, &successful);
		getObject(object, GL_INFO_LOG_LENGTH, &logLength);
		if (!successful) {
			std::vector<char> message(logLength, '\0');
			glGetShaderInfoLog(object, logLength, {}, message.data());
			error("{}: Unable to {}.\n{}", name, action, message.data());
			if (message.empty()) error("(no diagnostic)\n");
		}
	}

	void setupScene() {
		// Set up shaders.
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, {});
		glCompileShader(vertexShader);

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, {});
		glCompileShader(fragmentShader);

		m_shaderProgram = glCreateProgram();
		glAttachShader(m_shaderProgram, vertexShader);
		glAttachShader(m_shaderProgram, fragmentShader);
		glLinkProgram(m_shaderProgram);

		// Check for errors.
		checkObject<Shader>(vertexShader, "Vertex shader");
		checkObject<Shader>(fragmentShader, "Fragment shader");
		checkObject<Program>(m_shaderProgram, "Shader program");

		// TODO: Maybe handle this with RAII?
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		// Set up geometry.
		std::vector<GLfloat> vertices {
		//	   X     Y     Z
		  	 0.5,  0.5,  0.0,
		  	 0.5, -0.5,  0.0,
		  	-0.5, -0.5,  0.0,
		  	-0.5,  0.5,  0.0,
		};

		std::vector<GLuint> indices {
			0, 1, 3,
			1, 2, 3,
		};

		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(
			GL_ARRAY_BUFFER,
			vertices.size() * sizeof(GLfloat), vertices.data(),
			GL_STATIC_DRAW
		);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			indices.size() * sizeof(GLuint), indices.data(),
			GL_STATIC_DRAW
		);

		glVertexAttribPointer(
			0, 3,
			GL_FLOAT, GL_FALSE,
			3 * sizeof(GLfloat), (GLvoid*)0
		);
		glEnableVertexAttribArray(0);
	}
};
