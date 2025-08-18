#include <iostream>
#include <network/network.h>
#include <game/game_common.h>
#include "game/game.h"
#include "render/renderer.h"
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <thread>
#include <mutex>
#include <queue>
#include <fstream>

#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#include <ft2build.h>
#include FT_FREETYPE_H  

bool load_shader(const std::string& vertex_path, const std::string& fragment_path, unsigned int* program_ptr);

network::ring_buffer net_buffer(NETBUFLEN);
std::queue<network::message<game::client_action>> messages_to_send;

const int WIN_WIDTH = 1920;
const int WIN_HEIGHT = 1080;

struct rect
{
    double x, y, w, h;

    inline bool is_inside(double x, double y) const
    {
        return x >= this->x && x <= this->x + this->w && y >= this->y && y <= this->y + this->h;
    }
};

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
        network::message<game::server_action> message;
        while(network::next_message(net_buffer, &message))
        {
            // Parse Message
            switch(message.get_type())
            {
                case game::server_action::DISCONNECT:
                    std::cout << "Disconnect" << std::endl;
                    socket.close();
                    connected = false;
                break;
                case game::server_action::MESSAGE:
                    std::cout << "SERVER: " << message.get_payload().data() << std::endl;
                break;
                case game::server_action::CARD:
                {
                    game::card* c = (game::card*)&message.get_payload()[0];
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
            network::message<game::client_action> message = messages_to_send.front();
            messages_to_send.pop();
            // Send message over
            /*
                asio::send(server_socket, message.serialize());
            */
        }
    //endtick
    }
}


enum mouse_buttons
{
    LEFT,
    RIGHT,
    MIDDLE
};

enum button_states
{
    RELEASED,
    PRESSED,
    HELD
};

button_states buttons[3];

void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
    if(action == GLFW_PRESS) buttons[button] = button_states::PRESSED;
    else if(action == GLFW_RELEASE) buttons[button] = button_states::RELEASED;
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

    // Set up batch buffers for text
    unsigned int text_vao, text_vbo, text_ebo;
    glGenVertexArrays(1, &text_vao);
    glGenBuffers(1, &text_vbo);
    glGenBuffers(1, &text_ebo);

    glBindVertexArray(text_vao);

    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    glBufferData(GL_ARRAY_BUFFER, 1000 * sizeof(render::character_vertex), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(render::character_vertex), (void*)offsetof(render::character_vertex, pos));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(render::character_vertex), (void*)offsetof(render::character_vertex, tex));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1000 * 6 * sizeof(unsigned int), nullptr, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Create and populate font atlas for basic ascii characters

    const int atlas_dimension = 2048;

    unsigned int font_atlas;
    glGenTextures(1, &font_atlas);
    glBindTexture(GL_TEXTURE_2D, font_atlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas_dimension, atlas_dimension, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    FT_Library ft;
    if(FT_Init_FreeType(&ft))
    {
        std::cout << "Faild to init freetype" << std::endl;
        exit(EXIT_FAILURE);
    }

    FT_Face face;
    if(FT_New_Face(ft, "../assets/fonts/Roboto-Black.ttf", 0, &face))
    {
        std::cout << "Couldn't load font" << std::endl;
        exit(EXIT_FAILURE);
    }

    FT_Set_Pixel_Sizes(face, 0, 64);

    std::map<char, render::character_info> characters;

    stbrp_context context;
    stbrp_node nodes[atlas_dimension];
    stbrp_init_target(&context, atlas_dimension, atlas_dimension, nodes, atlas_dimension);

    // Go through adding all chars to the atlas
    for(unsigned char c = 0; c < 128; c++)
    {
        FT_GlyphSlot slot = face->glyph;

        if(FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "Failed to load glyph" << std::endl;
            exit(EXIT_FAILURE);
        }

        FT_Render_Glyph(slot, FT_RENDER_MODE_SDF);

        stbrp_rect r;
        r.w = face->glyph->bitmap.width;
        r.h = face->glyph->bitmap.rows;

        stbrp_pack_rects(&context, &r, 1);

        // Need to figure out how to pack these into this or whatever
        glTexSubImage2D(GL_TEXTURE_2D, 0, r.x, r.y, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        characters[c] = render::character_info{r.x, r.y, face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap_left, face->glyph->bitmap_top, face->glyph->advance.x};
    }


    std::string render_text = "This is some text, please work!";

    int total_indices = 0;

    // Where we want to render the string (given in percents)
    // Could just specify in pixels but later on when we have nested stuff we don't want to do that
    float xperc = 0.0;
    float yperc = 0.5;

    float scale = 1.0f;

    float xpixel = xperc * WIN_WIDTH;
    float ypixel = yperc * WIN_HEIGHT;

    for(int i = 0; i < render_text.length(); i++)
    {
        render::character_info info = characters[render_text[i]];

        std::cout << "Width: " << info.width << " Height: " << info.height << std::endl;
        std::cout << "X-Bearing: " << info.bearingx << " Y-Bearing: " << info.bearingy << std::endl;
        std::cout << "Advance: " << info.advance / 64.0 << std::endl;

        float x = 2.0 * ( (xpixel + info.bearingx * scale) / WIN_WIDTH ) - 1.0;
        float y = 2.0 * ( (ypixel - (info.height - info.bearingy) * scale) / WIN_HEIGHT) - 1.0;

        float w = 2.0 * (info.width * scale) / WIN_WIDTH;
        float h = 2.0 * (info.height * scale) / WIN_HEIGHT;

        float atlas_x = info.atlas_offsetx / (float)atlas_dimension;
        float atlas_y = info.atlas_offsety / (float)atlas_dimension;

        float atlas_w = info.width / (float)atlas_dimension;
        float atlas_h = info.height / (float)atlas_dimension;

        // TODO: Figure out vertex info from glyph info
        render::character_vertex vertices[] = 
        {
            {{x, y}, {atlas_x, atlas_y + atlas_h}},
            {{x + w, y},  {atlas_x + atlas_w, atlas_y + atlas_h}},
            {{x + w, y + h},   {atlas_x + atlas_w, atlas_y}},
            {{x, y + h},  {atlas_x, atlas_y}}
        };

        unsigned int i_offset = i * 4;

        unsigned int indices[] = 
        {
            i_offset, i_offset + 1, i_offset + 2,
            i_offset + 2, i_offset + 3, i_offset
        };


        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, i * 4 * sizeof(render::character_vertex), sizeof(vertices), vertices);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text_ebo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, i * 6 * sizeof(unsigned int), sizeof(indices), indices);
        total_indices += 6;
        xpixel += (info.advance >> 6) * scale;
    }


    glfwSetMouseButtonCallback(window, mouse_callback);

    glClearColor(0.3, 0.3, 0.3, 1.0);

    //std::thread thread(network_main);

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

    //rect r = { 1920 / 2, 1080 / 2, 1000, 1000};

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Update input
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        /*if(buttons[mouse_buttons::LEFT] == button_states::PRESSED && r.is_inside(xpos, ypos))
        {
            std::cout << "Mouse inside rectangle" << std::endl;
        }*/


        // After updating input and doing all logic, run this

        // Go through all buttons and assume they are held if they are pressed
        // Next frame when we poll inputs if the state didn't change (we are still holding the button) then it will be button_states::HELD
        // If we released the button then in the callback the state would be changed to button_states::RELEASED and all is good with the world.
        for(int i = 0; i < 3; i++)
        {
            if(buttons[i] == button_states::PRESSED) buttons[i] = button_states::HELD;
        }


        // Render
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(text_vao);
        
        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font_atlas);

        glDrawElements(GL_TRIANGLES, total_indices, GL_UNSIGNED_INT, nullptr);
        
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }


    glfwDestroyWindow(window);
    glfwTerminate();

    //thread.join();

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

    /*unsigned int x_tex;
    glGenTextures(1, &x_tex);
    glBindTexture(GL_TEXTURE_2D, x_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/