#include <iostream>
#include <network/network.h>
#include <game/game.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <thread>
#include <mutex>
#include <queue>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool load_shader(const std::string& vertex_path, const std::string& fragment_path, unsigned int* program_ptr);

Network::RingBuffer net_buffer(NETBUFLEN);
std::queue<Network::Message<ClientAction>> messages_to_send;

const int WIN_WIDTH = 1920;
const int WIN_HEIGHT = 1080;

void network_main()
{
    asio::io_context context;

    // Creates the endpoints for a given ip and port
    asio::ip::tcp::resolver resolver(context);
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "8080");
    
    asio::ip::tcp::socket socket(context);

    try
    {
        // Connect to the given endpoint with the given socket
        asio::connect(socket, endpoints);
    }
    catch(std::exception e)
    {
        std::cout << e.what() << std::endl;
        return;
    }


    // This will be on it's own network thread. Game data will be shared between render and network threads
    // Every tick 
    bool connected = true;
    while(connected)
    {
        char buf[256] = {};
        std::error_code error;
        size_t len = socket.read_some(asio::buffer(buf), error);
        if(len > 0) net_buffer.write_bytes(buf, len);

        if(error == asio::error::eof)
        {}
        else if(error)
        {
            throw std::system_error(error);
        }

        // Get all message
        Network::Message<ServerAction> message;
        while(Network::NextMessage(net_buffer, &message))
        {
            // Parse Message
            switch(message.get_type())
            {
                case ServerAction::DISCONNECT:
                    std::cout << "Disconnect" << std::endl;
                    socket.close();
                    connected = false;
                break;
                case ServerAction::MESSAGE:
                    std::cout << "SERVER: " << message.get_payload().data() << std::endl;
                break;
                case ServerAction::CARD:
                {
                    Card* c = (Card*)&message.get_payload()[0];
                    std::cout << "Card [ Suit: " << (uint32_t)c->rank << " Suit: " << (uint32_t)c->suit << "]" << std::endl;
                }
                break;
                default:
                    std::cout << "Got Default Message" << std::endl;
                break;
            }
        }
        // Send all pending messages
        while(!messages_to_send.empty())
        {
            Network::Message<ClientAction> message = messages_to_send.front();
            messages_to_send.pop();
            // Send message over
            /*
                asio::send(server_socket, message.serialize());
            */
        }
    //endtick
    }
}

int main()
{
    glfwInit();
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Poker", nullptr, nullptr);

    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        exit(EXIT_FAILURE);
    }

    glClearColor(0.3, 0.3, 0.3, 1.0);

    std::thread thread(network_main);

    float vertices[] = 
    {
        -1.0, -1.0,     0.0, 0.0,
        1.0, -1.0,      1.0, 0.0,
        1.0, 1.0,       1.0, 1.0,
        -1.0, 1.0,      0.0, 1.0
    };

    unsigned int indices[] = 
    {
        0, 1, 2,
        2, 3, 0
    };


    unsigned int vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Shader code here
    unsigned int program;
    if(!load_shader("../client/shader/default.vert", "../client/shader/default.frag", &program))
    {
        std::cout << "Failed to load shader" << std::endl;
        exit(EXIT_FAILURE);
    }

    stbi_set_flip_vertically_on_load(true);

    int width, height, comp;
    unsigned char* data = stbi_load("../assets/poker_cards/cards.png", &width, &height, &comp, 0);

    unsigned int texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vao);
        
        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
    }


    glfwDestroyWindow(window);
    glfwTerminate();

    thread.join();

    exit(EXIT_FAILURE);
}

std::string read_file(const std::string& path)
{
    std::ifstream file;
    file.open(path, std::ios::in);

    if(!file.is_open())
    {
        std::cout << "failed to open file" << std::endl;

        return "";
    }

    std::string str;
    std::string line;

    while(std::getline(file, line))
    {
        str += line + "\n";
    }

    file.close();

    return str;
}

bool load_shader(const std::string& vertex_path, const std::string& fragment_path, unsigned int* program_ptr)
{
    const int MAX_LOG = 512;
    int success;
    char log[MAX_LOG];

    std::string vertex_str = read_file(vertex_path);

    const char* const vertex_src = vertex_str.c_str();

    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_src, nullptr);
    glCompileShader(vertex_shader);

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader, MAX_LOG, nullptr, log);
        std::cerr << "Vertex: " << std::endl;
        std::cerr << log << std::endl;
        glDeleteShader(vertex_shader);
        return false;
    }

    std::string fragment_str = read_file(fragment_path);

    const char* const fragment_src = fragment_str.c_str();

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_src, nullptr);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment_shader, MAX_LOG, nullptr, log);
        std::cerr << "Fragment: " << std::endl;
        std::cerr << log << std::endl;
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return false;
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program, MAX_LOG, nullptr, log);
        std::cerr << "Linker: " << std::endl;
        std::cerr << log << std::endl;
        glDetachShader(program, vertex_shader);
        glDetachShader(program, fragment_shader);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(program);
        return false;
    }

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    *program_ptr = program;

    return true;
}