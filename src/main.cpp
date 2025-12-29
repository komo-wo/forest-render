/**
* Starter code for CECS 449 final project.
* Major modules:
*	Mesh: an OpenGL VAO associated with one or more textures. Transfers vertices from RAM to VRAM. (COMPLETE)
*   Object3D: pairs a Mesh with world-space parameters (position, orientation, scale), and methods to manipulate
*			  the object in world space.
*			  Object3D is hierarchical; a "hierarchical mesh" is actually a tree of Object3Ds, which render
*			  recursively. (COMPLETE)
*   AssimpImport: loads an assimp model file into an hierarchical Object3D. (COMPLETE)
*   Animator: a list of animations to apply over given time intervals.
*   Main: initializes a Scene, advances Animators, and renders objects in the scene.
*/
#include <glad/glad.h>
#include <iostream>
#include <filesystem>
#include <numbers>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <SFML/Graphics.hpp>

#include "AssimpImport.h"
#include "Mesh.h"
#include "SceneObject.h"
#include "ShaderProgram.h"

#define M_PI std::numbers::pi_v<float>

// We use a structure to track all the elements of a scene, including a list of objects,
// a list of animators, and a shader program to use to render those objects.
struct Scene {
	ShaderProgram program{};
	std::vector<SceneObject> objects{};
};

/**
 * @brief Constructs a shader program that applies the Phong reflection model.
 */
ShaderProgram phongLightingShader() {
	ShaderProgram shader{};
	try {
		shader.load("shaders/light_perspective.vert", "shaders/lighting.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return shader;
}

/**
 * @brief Constructs a shader program that performs texture mapping with no lighting.
 */
ShaderProgram texturingShader() {
	ShaderProgram shader{};
	try {
		shader.load("shaders/texture_perspective.vert", "shaders/texturing.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return shader;
}

/**
 * @brief Loads an image from the given path into an OpenGL texture.
 */
Texture loadTexture(const std::filesystem::path& path, const std::string& samplerName = "baseTexture") {
	StbImage i{};
	i.loadFromFile(path.string());
	return Texture::loadImage(i, samplerName);
}

Scene prayer() {
	Scene scene{ phongLightingShader() };
		// house
		auto house{ assimpLoad("../../../models/mushroom/mushroom.gltf", true) };
		house.position = glm::vec3{ 7, -1, 0 }; 
		house.scale = glm::vec3{ 9, 9, 9 };      
		scene.objects.push_back(std::move(house));

		//stump
		auto stump{ assimpLoad("../../../models/stump/stump.gltf", true) };
		stump.position = glm::vec3{ 9, -6, -23 };
		stump.scale = glm::vec3{ .025, .025, .025 };
		scene.objects.push_back(std::move(stump));

		//mushies 
		auto mushies{ assimpLoad("../../../models/mushies/mushies.gltf", true) };
		mushies.position = glm::vec3{ -5, -.6, -4 };
		mushies.scale = glm::vec3{ 1, 1, 1 };
		scene.objects.push_back(std::move(mushies));

		// tree
		auto tree{ assimpLoad("../../../models/tree/tree.gltf", true) };
		tree.position = glm::vec3{ 22, -6, 2 };
		tree.scale = glm::vec3{ 5, 5, 5 };
		scene.objects.push_back(std::move(tree));

		// fairy
		auto fairy{ assimpLoad("../../../models/fairy/fairy.gltf", true) };
		fairy.position = glm::vec3{ 0.8f, 2.9f, 0.5f };
		fairy.scale = glm::vec3{ .2f, .2f, .2f };
		scene.objects.push_back(std::move(fairy));

	return scene;
}


int main() {

	std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;

	// Initialize the window and OpenGL.
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.majorVersion = 3;
	settings.minorVersion = 3;

	sf::Window window{
		sf::VideoMode::getFullscreenModes().at(0), "Modern OpenGL",
		sf::Style::Resize | sf::Style::Close,
		sf::State::Windowed, settings
	};

	window.setMouseCursorVisible(false);

	if (!gladLoadGL()) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

	Scene myScene = prayer();
	myScene.program.activate();

	// Camera setup
	glm::vec3 cameraPos{ 0.0f, 1.3f, 5.0f };
	glm::vec3 cameraFront{ 0.0f, 0.0f, -1.0f };
	glm::vec3 cameraUp{ 0.0f, 1.0f, 0.0f };

	float speed = 0.08f;
	float yaw = -90.0f;      // Horizontal rotation (looking down -Z initially)
	float pitch = 0.0f;      // Vertical rotation (0 = looking at horizon)
	float sensitivity = 0.1f;

	bool firstMouse = true;
	sf::Vector2i lastMousePos;

	glm::mat4 projection =
		glm::perspective(glm::radians(45.0f),
			static_cast<float>(window.getSize().x) /
			static_cast<float>(window.getSize().y),
			0.1f, 500.0f); 

	sf::Clock c;

	int frameCount = 0;
	while (window.isOpen()) {
		frameCount++;

		// Frame time for smooth movement
		float deltaTime = c.restart().asSeconds();
		float frameSpeed = speed * deltaTime * 60.0f;

		// Calculate right vector before event loop
		glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));

		// Check for window events
		while (const std::optional event{ window.pollEvent() }) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
		}

		// Handle keyboard input (outside event loop for smooth movement)
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
			cameraPos += frameSpeed * cameraFront;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
			cameraPos -= frameSpeed * cameraFront;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
			cameraPos -= frameSpeed * right;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
			cameraPos += frameSpeed * right;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape)) {
			window.close();
		}

#ifdef LOG_FPS
		// FPS calculation.
		std::cout << 1 / deltaTime << " FPS " << std::endl;
#endif		
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		if (firstMouse) {
			lastMousePos = mousePos;
			firstMouse = false;
		}

		// Calculate mouse offset from center
		float xOffset = static_cast<float>(mousePos.x - lastMousePos.x) * sensitivity;
		float yOffset = static_cast<float>(lastMousePos.y - mousePos.y) * sensitivity; // Reversed Y

		lastMousePos = mousePos;

		yaw += xOffset;
		pitch += yOffset;

		// Clamp pitch to prevent screen flip
		pitch = glm::clamp(pitch, -89.0f, 89.0f);

		// Calculate new camera front vector with both yaw and pitch
		glm::vec3 front;
		front.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
		front.y = glm::sin(glm::radians(pitch));
		front.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);

		// Update view matrix
		glm::mat4 view = glm::lookAt(
			cameraPos,
			cameraPos + cameraFront,
			cameraUp
		);

		// Clear buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set uniforms for main shader and render scene objects
		myScene.program.activate();
		myScene.program.setUniform("view", view);
		myScene.program.setUniform("projection", projection);
		myScene.program.setUniform("cameraPos", cameraPos);

		// Material properties: ambient, diffuse, specular, shininess
		myScene.program.setUniform("material", glm::vec4{ 0.2f, 0.8f, 0.4f, 32 });
		myScene.program.setUniform("ambientColor", glm::vec3{ 0.65f, 0.65f, 0.65f });
		myScene.program.setUniform(
			"directionalLight",
			glm::normalize(glm::vec3{ 0.3f, 1.0f, 0.7f })
		);
		myScene.program.setUniform("directionalColor", glm::vec3{ 1.0f, 1.0f, 1.0f });

		myScene.program.setUniform(
			"pointLight.position",
			glm::vec3{ 0.8f, 3.3f, 0.5f }
		);
		myScene.program.setUniform("pointLight.color", glm::vec3(1.0f));
		myScene.program.setUniform("pointLight.constant", 1.0f);
		myScene.program.setUniform("pointLight.linear", 0.09f);
		myScene.program.setUniform("pointLight.quadratic", 0.032f);

		// Render scene objects
		for (auto& o : myScene.objects) {
			o.drawObject(myScene.program);

		}

		window.display();
	}

	return 0;
}