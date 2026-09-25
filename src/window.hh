#pragma once
#include "util.hh"
#include <glad/glad.h> // Must be included before GLFW.
#include <GLFW/glfw3.h>

// Programs right now can only have one monolithic window. When this window
// closes, everything closes.
template <class AppState=std::monostate>
class Window {
	GLFWwindow* handle_m {};
	AppState appState_m {};

	GLuint shaderProgram_m {};
	GLuint VBO_m {}, VAO_m {}, EBO_m {};

public:
	Window(
		unsigned width, unsigned height, std::string title,
		void (* setup     ) (Window<AppState>&, AppState&),
		void (* renderLoop) (Window<AppState>&, AppState&),
		AppState appState={}
	) {
		appState_m = appState;
		setupErrorLog();
		setupWindow(width, height, title);
		setupObjects();
		setup(*this, appState_m);
		while (!glfwWindowShouldClose(handle_m)) {
			renderLoop(*this, appState_m);
		}
	}

	~Window() {
		glDeleteVertexArrays(1, &VAO_m);
		glDeleteBuffers(1, &VBO_m);
		glDeleteBuffers(1, &EBO_m);
		glDeleteProgram(shaderProgram_m);
		glfwTerminate();
	}

	void close() {
		glfwSetWindowShouldClose(handle_m, true);
	}

	// Getters
	GLFWwindow* handle() const { return handle_m; }
	AppState& appState() /* */ { return appState_m; }
	AppState& appState() const { return appState_m; }

	GLuint shaderProgram() const { return shaderProgram_m; };
	GLuint VBO() const { return VBO_m; };
	GLuint VAO() const { return VAO_m; };
	GLuint EBO() const { return EBO_m; };

	// Setters
	void vertices(std::vector<GLfloat>& vs) {
		glBindBuffer(GL_ARRAY_BUFFER, VBO_m);
		glBufferData(GL_ARRAY_BUFFER,
			vs.size() * sizeof(GLfloat),
			vs.data(), GL_STATIC_DRAW
		);
	}

	void indices(std::vector<GLuint>& is) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_m);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			is.size() * sizeof(GLuint),
			is.data(), GL_STATIC_DRAW
		);
	}

	// GLFW specific
	template <class ... Args> using CallbackFn = void (GLFWwindow*, Args ... );
	template <class Fn> using CallbackSetterFn = Fn* (GLFWwindow*, Fn*);

	template <class ... Args>
	void bind(
		CallbackSetterFn<CallbackFn<Args ... >>& glfwCallbackSetter,
		auto const& userFunction
	) {
		static auto uf = userFunction;
		glfwCallbackSetter(handle_m,
			[] (GLFWwindow*, Args ... args) {
				uf(args ... );
			}
		);
	}

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

		// Create our window, keeping track of its handle.
		handle_m = glfwCreateWindow(width, height, title.c_str(), {}, {});
		if (!handle_m) exit("glfwCreateWindow: Unable to create window.\n");
		glfwMakeContextCurrent(handle_m);

		// Load in GLAD for normal looking GL function calls.
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			exit("gladLoadGLLoader: Unable to start GLAD.\n");
		}

		// Set our width and height, and respond when a user resizes.
		glViewport(0, 0, width, height);
		bind(glfwSetFramebufferSizeCallback,
			[&] (int newWidth, int newHeight) {
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

		shaderProgram_m = glCreateProgram();
		glAttachShader(shaderProgram_m, vertexShader);
		glAttachShader(shaderProgram_m, fragmentShader);
		glLinkProgram(shaderProgram_m);

		// Check for errors.
		checkObject<Shader>(vertexShader, "Vertex shader");
		checkObject<Shader>(fragmentShader, "Fragment shader");
		checkObject<Program>(shaderProgram_m, "Shader program");

		// TODO: Maybe handle this with RAII?
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		// Set up geometry objects.
		glGenVertexArrays(1, &VAO_m);
		glGenBuffers(1, &VBO_m);
		glGenBuffers(1, &EBO_m);

		glBindVertexArray(VAO_m);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_m);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_m);

		// Define position attribute.
		glVertexAttribPointer(
			0, 3, GL_FLOAT, GL_FALSE,
			6 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat))
		);
		glEnableVertexAttribArray(0);

		// Define color attribute.
		glVertexAttribPointer(
			1, 3, GL_FLOAT, GL_FALSE,
			6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))
		);
		glEnableVertexAttribArray(1);
	}
};
