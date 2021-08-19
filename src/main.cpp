#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

#define TIMER_START 60.0

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};





struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    bool gameStart = false;
    double startTime;

    //glm::vec3 backpackPosition = glm::vec3(0.0f);
    //float backpackScale = 1.0f;
    PointLight pointLight;
    ProgramState()
    : camera(glm::vec3(-2.32,0.54,5.87)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
    << clearColor.g << '\n'
    << clearColor.b << '\n'
    << ImGuiEnabled << '\n'
    << camera.Position.x << '\n'
    << camera.Position.y << '\n'
    << camera.Position.z << '\n'
    << camera.Front.x << '\n'
    << camera.Front.y << '\n'
    << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
        >> clearColor.g
        >> clearColor.b
        >> ImGuiEnabled
        >> camera.Position.x
        >> camera.Position.y
        >> camera.Position.z
        >> camera.Front.x
        >> camera.Front.y
        >> camera.Front.z;
    }
}

ProgramState *programState;

//promenljive za kretanje lopte
float move_rotate = 0.0;
float move_ball_far = 0.0;


int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }


    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/6.1.skybox.vs", "resources/shaders/6.1.skybox.fs");
    Shader transpShader("resources/shaders/transparentobj.vs", "resources/shaders/transparentobj.fs");
    Shader kantaShader("resources/shaders/kanta.vs", "resources/shaders/kanta.fs");
    // load models
    Model ourModelPas("resources/objects/pas/13463_Australian_Cattle_Dog_v3.obj");
    Model ourModelLopta("resources/objects/ball/10536_soccerball_V1_iterations-2.obj");
    Model ourModelKutija("resources/objects/kutija/14028_Wood_Fruit_Crate_v1_l1.obj");
    Model ourModelPomorandza("resources/objects/pomorandza/10195_Orange-L2.obj");

    // svetla kocka

    Shader lightCubeShader("resources/shaders/light_cube.vs", "resources/shaders/light_cube.fs");






    glEnable(GL_DEPTH_TEST);

    // setting coordinates:

    // transparent background square
    float transparentVertices[] = {
            // positions         // texture coordinates (non-swapped because stbi flips picture when loading)
            0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  0.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  0.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  1.0f
    };


    // skybox
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    float verticesKanta[] = {
            // positions          // colors           // texture coords
            0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
            0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
            -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
            0,1,3,
            1,2,3
    };

    //kocka
    float CubeVertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    //kanta
    unsigned int kantaVBO, kantaVAO, kantaEBO;
    glGenVertexArrays(1, &kantaVAO);
    glGenBuffers(1, &kantaVBO);
    glGenBuffers(1, &kantaEBO);

    glBindVertexArray(kantaVAO);

    glBindBuffer(GL_ARRAY_BUFFER, kantaVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesKanta), verticesKanta, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kantaEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int lightCubeVAO, lightCubeVBO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glGenBuffers(1, &lightCubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);

    glBindVertexArray(lightCubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);



    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // ruza VAO, VBO
    unsigned int transparentRoseVAO, transparentRoseVBO;
    glGenVertexArrays(1, &transparentRoseVAO);
    glGenBuffers(1, &transparentRoseVBO);
    glBindVertexArray(transparentRoseVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentRoseVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);



    // texture loading
    stbi_set_flip_vertically_on_load(true);

    unsigned int transparentRoseTexture = loadTexture(FileSystem::getPath("resources/textures/belaRuza.png").c_str());
    unsigned  int kantaTexture = loadTexture(FileSystem::getPath("resources/textures/kanta.png").c_str());

    //pozicije svetlecih kocki
    //NE MENJAJ !!!!!!!!!!!!!!!!!!!!
    //-----------------------------------------------
    glm::vec3 pointLightPositions[] = {
            glm::vec3(13.0f,1.8f,7.8f), //kutija
            glm::vec3(12.0f,-2.0f,-3.8f), //pas
            glm::vec3(6.0f ,-7.0 ,6.8f) //lopta
    };
    //------------------------------------------------
    stbi_set_flip_vertically_on_load(false);


    vector<std::string> faces
    {
        FileSystem::getPath("resources/textures/Fatbursparken/posx.jpg"),
        FileSystem::getPath("resources/textures/Fatbursparken/negx.jpg"),
        FileSystem::getPath("resources/textures/Fatbursparken/posy.jpg"),
        FileSystem::getPath("resources/textures/Fatbursparken/negy.jpg"),
        FileSystem::getPath("resources/textures/Fatbursparken/posz.jpg"),
        FileSystem::getPath("resources/textures/Fatbursparken/negz.jpg")
    };

    unsigned int cubemapTexture = loadCubemap(faces);


    transpShader.use();
    transpShader.setInt("texture1", 0);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    stbi_set_flip_vertically_on_load(true);



    // render loop
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);


        // render
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        // point lights
        PointLight& pointLight = programState->pointLight;
        pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
        pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
        pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
        pointLight.specular = glm::vec3(1.0, 1.0, 1.0);
        pointLight.constant = 0.8f;
        pointLight.linear = 0.0014f;
        pointLight.quadratic = 0.000007f;
        ourShader.use();


        // svetlo za kutiju i pomorandzu
        pointLight.position = glm::vec3(13.0f,1.8f,7.8f);
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", glm::vec3(0.1, 0.5 , sin(glfwGetTime()*1.5)+0.2));
        ourShader.setVec3("pointLight.diffuse", glm::vec3(0.1, sin(glfwGetTime()*1.5), 0.7));
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear + 0.05);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic + 0.05);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        // svetlo za psa
        pointLight.position = glm::vec3(12.0f,-2.0f,-3.8f);
        ourShader.setVec3("pointLight1.position", pointLight.position);
        ourShader.setVec3("pointLight1.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight1.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight1.specular", pointLight.specular);
        ourShader.setFloat("pointLight1.constant", pointLight.constant);
        ourShader.setFloat("pointLight1.linear", pointLight.linear);
        ourShader.setFloat("pointLight1.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);


        //svetlo za loptu
        glm::vec3 lopta_kordinate = glm::vec3(6.0f + move_ball_far * pow(glfwGetTime()/2,2),-7.0 + move_rotate * 3 * sin(glfwGetTime()),6.8f );
        pointLight.position = glm::vec3(lopta_kordinate + glm::vec3(0,3,0));
        ourShader.setVec3("pointLight2.position", pointLight.position);
        ourShader.setVec3("pointLight2.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight2.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight2.specular", pointLight.specular);
        ourShader.setFloat("pointLight2.constant", pointLight.constant);
        ourShader.setFloat("pointLight2.linear", pointLight.linear);
        ourShader.setFloat("pointLight2.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        //lampa
        ourShader.setVec3("spotLight.position", glm::vec3(programState->camera.Position));
        ourShader.setVec3("spotLight.direction", programState->camera.Front);
        ourShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        ourShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("spotLight.constant", 0.5f);
        ourShader.setFloat("spotLight.linear", 0.03);
        ourShader.setFloat("spotLight.quadratic", 0.032);
        ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));


        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // rendering loaded models


        //model matrica
        glm::mat4 model = glm::mat4(1.0f);




        //PAS
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(12.0f,-8.0f,-5.8f));
        model = glm::rotate(model,glm::radians(-90.0f),glm::vec3(1.0,0.0,0.0));
        model = glm::scale(model, glm::vec3(0.19f,0.19f,0.19f));
        ourShader.setMat4("model", model);
        ourModelPas.Draw(ourShader);


        //LOPTA
        model = glm::mat4(1.0f);
        model = glm::translate(model, lopta_kordinate);
        model = glm::rotate(model,move_rotate * float(glfwGetTime()/2.0),glm::vec3(0.0,1.0,0.0));
        model = glm::scale(model, glm::vec3(0.1f,0.1f,0.1f));
        ourShader.setMat4("model", model);
        ourModelLopta.Draw(ourShader);

        //KUTIJA
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(14.0f,-5.0f,11.8f));
        model = glm::rotate(model,glm::radians(-90.0f),glm::vec3(1.0,0,0.0));
        model = glm::rotate(model,glm::radians(45.0f),glm::vec3(0.0,0.0,1.0));
        model = glm::scale(model, glm::vec3(0.03f,0.03f,0.03f));
        ourShader.setMat4("model", model);
        ourModelKutija.Draw(ourShader);

        //POMORANDZA
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(12.50f,-5.2f,11.8f));
        model = glm::rotate(model,glm::radians(-90.0f),glm::vec3(1.0,0,0.0));
        model = glm::rotate(model,glm::radians(45.0f),glm::vec3(0.0,0.0,1.0));
        model = glm::scale(model, glm::vec3(0.03f,0.03f,0.03f));
        ourShader.setMat4("model", model);
        ourModelPomorandza.Draw(ourShader);




        //RUZA

        transpShader.use();
        projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(9.5f,-8.50f,-7.8f));
        model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.0, 0.0, 1.0));
        model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.0, 1.0, 0.0));
        model = glm::scale(model, glm::vec3(2.5f, 2.5f, 2.5f));
        transpShader.setMat4("model", model);
        transpShader.setMat4("projection", projection);
        transpShader.setMat4("view", view);

        glBindVertexArray(transparentRoseVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentRoseTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);




        //KANTA
        glBindTexture(GL_TEXTURE_2D, kantaTexture);
        kantaShader.use();
        projection = glm::perspective(glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-10.5f, -10.6f, 32.0f));
        model = glm::scale(model,glm::vec3(6.0f, 6.0f, 6.0f));

        kantaShader.setMat4("model", model);
        kantaShader.setMat4("projection", projection);
        kantaShader.setMat4("view", view);
        glBindVertexArray(kantaVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);




        //---------------------------


        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);


        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < 3; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.08f)); // Make it a smaller cube
            lightCubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        //-----------------------------------------------

        // drawing skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;


    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
        printf("pritisnuli ste esc");
    }


    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        programState->camera.Position = glm::vec3(-2.32,0.54,5.87);

    //pocinje tapkanje lopte
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        move_rotate = 1;

    //lopta odskakuce
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        move_ball_far = 1;
        //move_rotate = 0;
    }



}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        if (!programState->gameStart) {
            programState->startTime = glfwGetTime();
        }
        programState->gameStart = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if(key == GLFW_KEY_SPACE && action == GLFW_PRESS && programState->gameStart) {
        double xpos = programState->camera.Front.x;
        double ypos = programState->camera.Front.y;
        double zpos = programState->camera.Front.z;


    }
}


unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}



