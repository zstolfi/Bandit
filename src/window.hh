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

	std::vector<GLfloat> m_vertices {};
	std::vector<GLuint> m_indices {};

public:
	Window(
		unsigned width, unsigned height, std::string title,
		void (* setup     )(Window&),
		void (* renderLoop)(Window&)
	) {
		setupErrorLog();
		setupWindow(width, height, title);
		setup(*this);
		setupObjects();
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
	GLuint EBO() const { return m_EBO; };

	std::vector<GLfloat>& vertices() { return m_vertices; }
	std::vector<GLuint>& indices() { return m_indices; }

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

	stdfs::path const vertexShaderPath = "vertex.glsl";

	stdfs::path const fragmentShaderPath = "fragment.glsl";

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

	auto loadSource(stdfs::path path) {
		auto result = std::string {};

		if (std::ifstream file {path}) {
			std::istreambuf_iterator<char> begin {file}, end {};
			result = {begin, end};
		}
		else error("loadSource: Unable to load {}.\n", path.native());

		return result;
	}

	void setupObjects() {
		// Set up shaders.
		std::string vertexShaderSource = loadSource(vertexShaderPath);
		std::string fragmentShaderSource = loadSource(fragmentShaderPath);
		char const* currentSource {};

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		currentSource = vertexShaderSource.c_str();
		glShaderSource(vertexShader, 1, &currentSource, {});
		glCompileShader(vertexShader);

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		currentSource = fragmentShaderSource.c_str();
		glShaderSource(fragmentShader, 1, &currentSource, {});
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

		// Set up geometry objects.
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(
			GL_ARRAY_BUFFER,
			m_vertices.size() * sizeof(GLfloat), m_vertices.data(),
			GL_STATIC_DRAW
		);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			m_indices.size() * sizeof(GLuint), m_indices.data(),
			GL_STATIC_DRAW
		);

		// Position attribute:
		glVertexAttribPointer(
			0, 3, GL_FLOAT, GL_FALSE,
			6 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat))
		);
		glEnableVertexAttribArray(0);

		// Color attribute:
		glVertexAttribPointer(
			1, 3, GL_FLOAT, GL_FALSE,
			6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))
		);
		glEnableVertexAttribArray(1);
	}
};
