#include <iostream>
#include <network/network.h>
#include <game/game.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <thread>
#include <mutex>
#include <queue>

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

    // Connect to the given endpoint with the given socket
    asio::connect(socket, endpoints);


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

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
    }


    glfwDestroyWindow(window);
    glfwTerminate();

    thread.join();

    exit(EXIT_FAILURE);
}