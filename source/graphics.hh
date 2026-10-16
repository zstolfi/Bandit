#pragma once
#include "math.hh"
#include "util.hh"
#include <glad/glad.h>

auto const Colors = std::vector<Color<3>> {
	{{0.9, 0.9, 0.9}}, // White
	{{0.9, 0.9, 0.2}}, // Yellow
	{{0.2, 0.3, 0.9}}, // Blue
	{{0.1, 0.9, 0.2}}, // Green
	{{0.9, 0.2, 0.2}}, // Red
	{{0.9, 0.5, 0.2}}, // Orange
};

auto const ColorsBackground = std::vector<Color<3>> {
	{{0.7, 0.7, 0.7}}, // Unsolved
	{{0.6, 0.7, 0.6}}, // Solved
};

class Shader {
	GLuint program_m {};

	auto static LogInfo() {
		return std::tuple {
			glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS
		};
	}

	class SubShader {
		friend Shader;
		GLuint id_m {};

		auto static LogInfo() {
			return std::tuple {
				glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS
			};
		}

	public:
		SubShader(std::span<char const> source, GLenum shaderType)
		:	id_m {glCreateShader(GL_VERTEX_SHADER)} {
			auto string = source.data();
			glShaderSource(id_m, 1, &string, {});
			glCompileShader(id_m);
		}

		~SubShader() {
			glDeleteShader(id_m);
		}

		GLuint id() const { return id_m; }
	};

public:
	Shader() = default;

	Shader(
		std::span<char const> vertexSource,
		std::span<char const> fragmentSource
	) {
		SubShader vertexShader {vertexSource, GL_VERTEX_SHADER};
		SubShader fragmentShader {fragmentSource, GL_FRAGMENT_SHADER};

		program_m = glCreateProgram();
		glAttachShader(program_m, vertexShader.id());
		glAttachShader(program_m, fragmentShader.id());
		glLinkProgram(program_m);

		// Check for errors.
		verify(vertexShader, "Vertex shader", "compile");
		verify(fragmentShader, "Fragment shader", "compile");
		verify(*this, "Shader program", "link");
	}

	~Shader() {
		glDeleteProgram(program_m);
	}

	GLuint id() const { return program_m; }

	void static verify(auto object, std::string name, std::string action) {
		auto [getter, logGetter, status] = decltype(object)::LogInfo();

		GLint successful {}, logLength {};
		getter(object.id(), status, &successful);
		getter(object.id(), GL_INFO_LOG_LENGTH, &logLength);
		if (!successful) {
			std::vector<char> message(logLength, '\0');
			logGetter(object.id(), logLength, {}, message.data());
			error("{}: Unable to {}.\n{}", name, action, message.data());
			if (message.empty()) error("(no diagnostic)\n");
		}
	}
};

class Model {
	struct Vertex_t {
		Coord<3, float> position {};
		Color<3, float> color {};
	};

	std::vector<Vertex_t> vertices_m {};
	GLuint VBO_m {}, VAO_m {};

public:
	using Vertex = Vertex_t;
	Model() = default;

	~Model() {
		glDeleteVertexArrays(1, &VAO_m);
		glDeleteBuffers(1, &VBO_m);
	}

	auto const& vertices() const { return vertices_m; }
	GLuint VBO() const { return VBO_m; };
	GLuint VAO() const { return VAO_m; };

	void vertices(std::span<Vertex_t const> vs) {
		glBindBuffer(GL_ARRAY_BUFFER, VBO_m);
		glBufferData(GL_ARRAY_BUFFER,
			vs.size() * sizeof(Vertex_t),
			vs.data(), GL_STATIC_DRAW
		);
	}
};

struct Scene {
	Color<3> backgroundColor {};
	std::vector<std::shared_ptr<Model>> models {};
};

class Graphics {
	std::array<unsigned, 2> const* size_m {};
	Scene const* scene_m;
	Shader shader_m {};

public:
	Graphics() = default;

	Graphics(auto* procAddress, std::array<unsigned, 2> const* size) {
		// Load in GLAD for normal looking GL function calls.
		if (!gladLoadGLLoader((GLADloadproc)procAddress)) {
			exit("gladLoadGLLoader: Unable to start GLAD.\n");
		}
		size_m = size;

		shader_m = {
			loadSource(vertexShaderPath),
			loadSource(fragmentShaderPath),
		};

		// Define position attribute.
		glVertexAttribPointer(
			0, decltype(Model::Vertex::position)::Dimension(),
			GL_FLOAT, GL_FALSE, sizeof(Model::Vertex),
			(GLvoid*)offsetof(Model::Vertex, position)
		);
		glEnableVertexAttribArray(0);

		// Define color attribute.
		glVertexAttribPointer(
			1, decltype(Model::Vertex::color)::Dimension(),
			GL_FLOAT, GL_FALSE, sizeof(Model::Vertex),
			(GLvoid*)offsetof(Model::Vertex, color)
		);
		glEnableVertexAttribArray(1);

		glEnable(GL_DEPTH_TEST);
	};

	Shader shader() const { return shader_m; };

	void scene(Scene const& scene) {
		scene_m = &scene;
	}

	template <class ... Args>
	void uniform(auto& setter, std::string const& name, Args const& ... args) {
		setter(glGetUniformLocation(shader_m.id(), name.c_str()), args ... );
	}

	stdfs::path static inline const vertexShaderPath = "vertex.glsl";
	stdfs::path static inline const fragmentShaderPath = "fragment.glsl";

	auto static loadSource(stdfs::path path) {
		auto result = std::string {};
		if (std::ifstream file {path}) {
			std::istreambuf_iterator<char> begin {file}, end {};
			result = {begin, end};
		}
		else error("loadSource: Unable to load {}.\n", path.native());
		return result;
	}

	void draw() const {
		glViewport(0, 0, (*size_m)[0], (*size_m)[1]);
		glUseProgram(shader_m.id());
		if (!scene_m) return;

		glClearColor(
			scene_m->backgroundColor.r(),
			scene_m->backgroundColor.g(),
			scene_m->backgroundColor.b(),
			1.0
		);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// TODO: Camerawork.

		for (auto model: scene_m->models) {
			glBindVertexArray(model->VAO());
			glBindBuffer(GL_ARRAY_BUFFER, model->VBO());
			glDrawArrays(GL_TRIANGLES, 0, model->vertices().size());
		}
	}
};
