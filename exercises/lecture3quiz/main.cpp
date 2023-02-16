#include <ituGL/core/DeviceGL.h>
#include <ituGL/application/Window.h>
#include <ituGL/geometry/VertexBufferObject.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/geometry/VertexAttribute.h>
#include <ituGL/geometry/ElementBufferObject.h>
#include <iostream>
#include <array>
#include <glm/glm.hpp>

int buildShaderProgram();
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 512;
const unsigned int SCR_HEIGHT = 512;
const float speed = 1.0f;
float rotation = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    DeviceGL deviceGL;

    // glfw window creation
    // --------------------
    Window window(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL");
    if (!window.IsValid())
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        return -1;
    }

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    deviceGL.SetCurrentWindow(window);
    if (!deviceGL.IsReady())
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    int shaderProgram = buildShaderProgram();

    // Using std::array instead of regular arrays makes sure we don't access out of range
    std::array<float, 8 * 3> vertices 
    {
        0.5f, -0.5f,  0.5f, // 0
        0.5f,  0.5f,  0.5f, // 1
       -0.5f, -0.5f,  0.5f, // 2
       -0.5f,  0.5f,  0.5f, // 3

        0.5f, -0.5f, -0.5f, // 4
        0.5f,  0.5f, -0.5f, // 5
       -0.5f, -0.5f, -0.5f, // 6
       -0.5f,  0.5f, -0.5f  // 7
    };

    std::array<float, 12 * 3> colors
    {
        1.0f, 0.0f, 0.0f, 
        1.0f, 0.0f, 0.0f, 
        0.0f, 1.0f, 0.0f, 
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,

    };

    std::array<unsigned int, 12 * 3> indices
    {
        0, 2, 1,
        1, 2, 3,

        0, 4, 5,
        0, 1, 5,

        5, 4, 6,
        7, 6, 5,

        6, 7, 3,
        6, 2, 3,
    };

    std::array<unsigned int, 6 * 3> colIndices
    {


    };

    VertexBufferObject posVbo;
    VertexBufferObject colVbo;
    VertexArrayObject vao;
    ElementBufferObject ebo;

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    vao.Bind();

    posVbo.Bind();
    posVbo.AllocateData<float>(std::span(vertices));
    VertexAttribute position(Data::Type::Float, 3);
    vao.SetAttribute(0, position, 0, 0);

    colVbo.Bind();
    colVbo.AllocateData<float>(std::span(colors));
    VertexAttribute color(Data::Type::Float, 3);
    vao.SetAttribute(1, color, 0, 0);

    ebo.Bind();
    ebo.AllocateData<unsigned int>(std::span(indices));
    deviceGL.SetWireframeEnabled(true);
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    VertexBufferObject::Unbind();

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    VertexArrayObject::Unbind();

    // Now we can unbind the EBO as well
    ElementBufferObject::Unbind();

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // render loop
    // -----------
    while (!window.ShouldClose())
    {
        // input
        // -----
        processInput(window.GetInternalWindow());

        // render
        // ------
        deviceGL.Clear(Color(0.2f, 0.3f, 0.3f, 1.0f));

        // draw our first triangle
        glUseProgram(shaderProgram);
        vao.Bind(); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        int phi = glm::radians(rotation);
        //glm::mat3 rotZ = glm::mat3(cos(12), sin(12), 0, cos(12), -sin(12), 0, 0, 0, 1);
        glm::mat3 rotY = glm::mat3(cos(phi), 0, sin(phi), 0, 1, 0, -sin(phi), 0, cos(phi));
        glm::mat4 model = glm::mat4(rotY);

        unsigned int uniformID = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(uniformID, 1, GL_FALSE, &model[0][0]);

        //glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        // VertexArrayObject::Unbind(); // no need to unbind it every time 

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        window.SwapBuffers();
        deviceGL.PollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    // This is now done in the destructor of DeviceGL
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        rotation += speed;
    }
}

// build the shader program
// ------------------------
int buildShaderProgram() 
{
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aCol;\n"
        "out vec3 color;\n"
        "uniform mat4 model;\n"
        "void main()\n"
        "{\n"
        "   color = aCol;\n"
        "   gl_Position = model * vec4(aPos, 1.0);\n"
        "}\0";
    const char* fragmentShaderSource = "#version 330 core\n"
        "in vec3 color;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(color, 1.0f);\n"
        "}\n\0";

    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}
