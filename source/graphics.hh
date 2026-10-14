#pragma once
#include "math.hh"
#include "util.hh"
#include <glad/glad.h>

auto const Colors = std::vector<Color<3>> {
	{{0.9, 0.5, 0.2}}, // Orange
	{{0.9, 0.2, 0.2}}, // Red
	{{0.9, 0.9, 0.9}}, // White
	{{0.9, 0.9, 0.2}}, // Yellow
	{{0.2, 0.3, 0.9}}, // Blue
	{{0.1, 0.9, 0.2}}, // Green
};

auto const ColorsBackground = std::vector<Color<3>> {
	{{0.7, 0.7, 0.7}}, // Unsolved
	{{0.6, 0.7, 0.6}}, // Solved
};

class Model {
	std::vector<GLfloat> vertices_m {};

public:
	Model() = default;

	auto const& vertices() const { return vertices_m; }
};

struct Scene {
	Color<3> backgroundColor {};
	std::vector<std::shared_ptr<Model>> models {};
};

class Graphics {
	Scene scene_m;

	GLuint shaderProgram_m {};
	GLuint VBO_m {}, VAO_m {};

public:
	Graphics() {
		setupOpenGl();
	};

	~Graphics() {
		glDeleteVertexArrays(1, &VAO_m);
		glDeleteBuffers(1, &VBO_m);
		glDeleteProgram(shaderProgram_m);
	}

	GLuint shaderProgram() const { return shaderProgram_m; };
	GLuint VBO() const { return VBO_m; };
	GLuint VAO() const { return VAO_m; };

	void scene(Scene const&) {
//		glBindBuffer(GL_ARRAY_BUFFER, VBO_m);
//		glBufferData(GL_ARRAY_BUFFER,
//			vs.size() * sizeof(GLfloat),
//			vs.data(), GL_STATIC_DRAW
//		);
	}

	template <class ... Args>
	void uniform(auto& setter, std::string const& name, Args const& ... args) {
		setter(glGetUniformLocation(shaderProgram_m, name.c_str()), args ... );
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

	void setupOpenGl() {
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

		// Generate geometry objects.
		glGenVertexArrays(1, &VAO_m);
		glGenBuffers(1, &VBO_m);

		glBindVertexArray(VAO_m);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_m);

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

		glEnable(GL_DEPTH_TEST);
	}

	void draw() const {
		glUseProgram(shaderProgram_m);

		// TODO: Camerawork.

//		glBindVertexArray(VAO_m);
//		glDrawArrays(GL_TRIANGLES, 0, model.vertices().size()/6);
//		glBindVertexArray(0);
	}
};
