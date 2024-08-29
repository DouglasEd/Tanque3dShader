#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>


float cameraAngle = 0.0f;
float AngTorreX = 0.0f;
float AngTorreY = 0.0f;
float PosX =0.0f;
float PosZ =0.0f;

GLuint cylinderVAO, cylinderVBO, cylinderEBO;
GLuint wheelVAO, wheelVBO, wheelEBO;
std::vector<unsigned int> cylinderIndices;
std::vector<unsigned int> wheelIndices;

// Definição do VertexShader e FragmentShader
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 ourColor;

void main() {
    FragColor = vec4(ourColor, 1.0);
}
)";

// Compilação dos Shaders
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation error: " << infoLog << std::endl;
    }
    return shader;
}

// Criação do programa do shader
GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Program linking error: " << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}
//Função de criação do cilidro usado para o canhão
void createCylinder(std::vector<float>& vertices, std::vector<unsigned int>& indices, int numSegments = 36, float height = 1.0f, float radius = 1.0f) {
    float angleStep = 2.0f * 3.14159265f / numSegments;

    // Vertices for the cylinder
    for (int i = 0; i < numSegments; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        
        // Bottom circle
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(-height / 2.0f);
        vertices.push_back(1.0f);  // Red
        vertices.push_back(0.0f);  // Green
        vertices.push_back(0.0f);  // Blue

        // Top circle
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(height / 2.0f);
        vertices.push_back(0.0f);  // Red
        vertices.push_back(1.0f);  // Green
        vertices.push_back(0.0f);  // Blue
    }

    // Indices for the side faces
    for (int i = 0; i < numSegments; ++i) {
        int next = (i + 1) % numSegments;
        indices.push_back(i * 2);
        indices.push_back(next * 2);
        indices.push_back(i * 2 + 1);
        
        indices.push_back(i * 2 + 1);
        indices.push_back(next * 2);
        indices.push_back(next * 2 + 1);
    }
}
//Função pra criação do cilindro usado na roda
void createWheel(std::vector<float>& vertices, std::vector<unsigned int>& indices, int numSegments = 36, float radius = 0.5f, float width = 0.2f) {
    float angleStep = 2.0f * 3.14159265f / numSegments;

    // Center vertices (one for each side)
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(-width / 2.0f);
    vertices.push_back(1.0f);  // Red
    vertices.push_back(1.0f);  // Green
    vertices.push_back(1.0f);  // Blue

    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(width / 2.0f);
    vertices.push_back(1.0f);  // Red
    vertices.push_back(1.0f);  // Green
    vertices.push_back(1.0f);  // Blue

    // Outer vertices
    for (int i = 0; i < numSegments; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float y = radius * sin(angle);

        // Outer circle
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(-width / 2.0f);
        vertices.push_back(1.0f);  // Red
        vertices.push_back(0.0f);  // Green
        vertices.push_back(0.0f);  // Blue

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(width / 2.0f);
        vertices.push_back(0.0f);  // Red
        vertices.push_back(1.0f);  // Green
        vertices.push_back(0.0f);  // Blue
    }

    // Indices for the side faces
    for (int i = 0; i < numSegments; ++i) {
        int next = (i + 1) % numSegments;

        // Side faces
        indices.push_back(i * 2 + 2);
        indices.push_back(next * 2 + 2);
        indices.push_back(i * 2 + 3);
        
        indices.push_back(i * 2 + 3);
        indices.push_back(next * 2 + 2);
        indices.push_back(next * 2 + 3);

        // Fill the front face
        indices.push_back(0);
        indices.push_back(i * 2 + 2);
        indices.push_back(next * 2 + 2);

        // Fill the back face
        indices.push_back(1);
        indices.push_back(next * 2 + 3);
        indices.push_back(i * 2 + 3);
    }
}
// Função para configurar os Buffers das formas usadas
void setupBuffers(GLuint &VAO, GLuint &VBO, GLuint &cylinderVAO, GLuint &cylinderVBO, GLuint &cylinderEBO, std::vector<unsigned int>& cylinderIndices, GLuint &wheelVAO, GLuint &wheelVBO,GLuint &wheelEBO, std::vector<unsigned int>& wheelIndices) {
    GLfloat vertices[] = {
        // Positions          // Colors
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  // Vertex 1
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  // Vertex 2
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  // Vertex 3
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  // Vertex 4
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  // Vertex 5
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  // Vertex 6
         0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,  // Vertex 7
        -0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.5f   // Vertex 8
    };
    
    GLuint indices[] = {
        0, 1, 2, 2, 3, 0,  // Front
        4, 5, 6, 6, 7, 4,  // Back
        0, 1, 5, 5, 4, 0,  // Bottom
        2, 3, 7, 7, 6, 2,  // Top
        0, 3, 7, 7, 4, 0,  // Left
        1, 2, 6, 6, 5, 1   // Right
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);

    // Buffers do Cilindro do canhão
    std::vector<float> cylinderVertices;
    createCylinder(cylinderVertices, cylinderIndices);

    glGenVertexArrays(1, &cylinderVAO);
    glGenBuffers(1, &cylinderVBO);
    glGenBuffers(1, &cylinderEBO);

    glBindVertexArray(cylinderVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
    glBufferData(GL_ARRAY_BUFFER, cylinderVertices.size() * sizeof(float), &cylinderVertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinderEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinderIndices.size() * sizeof(unsigned int), &cylinderIndices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    //Buffers Roda
    std::vector<float> wheelVertices;
    createWheel(wheelVertices, wheelIndices);
    glGenVertexArrays(1, &wheelVAO);
    glGenBuffers(1, &wheelVBO);
    glGenBuffers(1, &wheelEBO);
    glBindVertexArray(wheelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wheelVBO);
    glBufferData(GL_ARRAY_BUFFER, wheelVertices.size() * sizeof(float), &wheelVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wheelEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, wheelIndices.size() * sizeof(unsigned int), &wheelIndices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}
//Funçaõ usada para desenhar os Cubos usados
void drawCube(GLuint shaderProgram, GLuint VAO, glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
    glUseProgram(shaderProgram);
    
    // Set uniforms
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
//Funçaõ usada para desenhar o cilindro usado no Canhão
void drawCylinder(GLuint shaderProgram, GLuint cylinderVAO, glm::mat4 model, glm::mat4 view, glm::mat4 projection, int numIndices) {
    glUseProgram(shaderProgram);
    
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    glBindVertexArray(cylinderVAO);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT){
        if (key == GLFW_KEY_D){
            AngTorreX -= 1.0f;
        }
        if (key == GLFW_KEY_A){
            AngTorreX += 1.0f;
        }
        if (key == GLFW_KEY_W){
            if(AngTorreY>-90){
                AngTorreY -= 1.0f;
            }
        }
        if (key == GLFW_KEY_S){
            if(AngTorreY<0){
                AngTorreY += 1.0f;
            }
        }
        if (key == GLFW_KEY_UP){
            PosZ += 0.1f;
        }
        if (key == GLFW_KEY_DOWN){
            PosZ -= 0.1f;
        }
        if (key == GLFW_KEY_LEFT){
           PosX += 0.1f;
        }
        if (key == GLFW_KEY_RIGHT){
            PosX-= 0.1f;
        }
    }
}
int main() {
    // Iniciação do GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //Criação da janela
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Cubes", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Iniciaçaõ do Glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    GLuint shaderProgram = createShaderProgram();

    GLuint VAO, VBO;
    GLuint cylinderVAO, cylinderVBO, cylinderEBO;
    GLuint wheelVAO, wheelVBO, wheelEBO;
    std::vector<unsigned int> cylinderIndices, wheelIndices;
    
    setupBuffers(VAO, VBO, cylinderVAO, cylinderVBO, cylinderEBO, cylinderIndices, wheelVAO, wheelVBO, wheelEBO, wheelIndices);
    
    glEnable(GL_DEPTH_TEST);

    glfwSetKeyCallback(window, key_callback);

    // Looping principal do Programa, onde é desenhado as formas e controlado a Camera
    while (!glfwWindowShouldClose(window)) {
        if(AngTorreX == 180||AngTorreX == -180){
            AngTorreX*=-1;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glm::mat4 model = glm::mat4(1.0f);
        //glm::mat4 view = glm::lookAt(glm::vec3(4.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        cameraAngle += 0.0001f;  // Aumente ou diminua este valor para ajustar a velocidade de rotação

    // Atualizar a matriz de visualização
    glm::mat4 view = glm::lookAt(
        glm::vec3(4.0f * cos(cameraAngle), 3.0f, 4.0f * sin(cameraAngle)),  // Posição da câmera
        glm::vec3(0.0f, 0.0f, 0.0f),  // Olhar para o centro da cena
        glm::vec3(0.0f, 1.0f, 0.0f)   // Definir o eixo "up" (para cima)
    );

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        
        // Corpo
        model = glm::translate(model, glm::vec3(0.0f+PosX, 0.0f, 0.0f+PosZ));
        model = glm::scale(model, glm::vec3(1.0f, 0.75f, 2.5f));  // Extend the front
        drawCube(shaderProgram, VAO, model, view, projection);

        //Esteira direita
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.6f+PosX, -0.30f, 0.0f+PosZ));
        model = glm::scale(model, glm::vec3(0.25f, 0.75f, 2.5f));
        drawCube(shaderProgram, VAO, model, view, projection);

        //Esteira Esquerda
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.6f+PosX, -0.30f, 0.0f+PosZ));
        model = glm::scale(model, glm::vec3(0.25f, 0.75f, 2.5f));
        drawCube(shaderProgram, VAO, model, view, projection);
        //Roda
        for(int i=0;i<3;i++){
            //Roda Direita
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.7f+PosX, -0.30f, 0.825f-(i*0.425*2)+PosZ));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.75f, 0.75f, 0.65f)); // Ajustar escala para rodas
            drawCylinder(shaderProgram, wheelVAO, model, view, projection, wheelIndices.size());
            //Roda Esquerda
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-0.7f+PosX, -0.30f, 0.825f-(i*0.425*2)+PosZ));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(0.0F), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.65f)); // Ajustar escala para rodas
            drawCylinder(shaderProgram, wheelVAO, model, view, projection, wheelIndices.size());
        }
        //torre
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f+PosX, 0.625f, -0.75f+PosZ));
        model = glm::rotate(model, glm::radians(AngTorreX), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(AngTorreY), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        drawCube(shaderProgram, VAO, model, view, projection);

        //Canhao
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f+PosX, 0.625f, 0.25f+PosZ));  // Adjust cannon position
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.625f * 1.5f));
        model = glm::rotate(model, glm::radians(AngTorreX), glm::vec3(0.0f, 1.0f, 0.0f));  // Rotate to align with turret
        model = glm::rotate(model, glm::radians(AngTorreY), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.625f * 1.5f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 1.5f));  // Adjust cylinder scale
        drawCylinder(shaderProgram, cylinderVAO, model, view, projection, cylinderIndices.size());
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    
    glfwTerminate();
    return 0;
}