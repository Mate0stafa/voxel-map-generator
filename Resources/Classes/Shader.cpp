#include "Shader.h"
#include "Lib/Glad/include/glad/glad.h"
#include <stdexcept>
#include <vector>

static std::vector<std::string> buildCandidates(const std::string& path) {
    std::vector<std::string> cands;

    auto isAbsolute = [](const std::string& p) {
#ifdef _WIN32
        return p.size() > 1 && (p[1] == ':' || (p.size() > 2 && p[0] == '\\' && p[1] == '\\'));
#else
        return !p.empty() && p[0] == '/';
#endif
    };

    auto containsSlash = [](const std::string& p) {
        return p.find('/') != std::string::npos || p.find('\\') != std::string::npos;
    };

    // 1) As-is
    cands.push_back(path);

    if (!isAbsolute(path)) {
        // 2) One directory up
        cands.push_back("../" + path);
        // 3) Two directories up
        cands.push_back("../../" + path);
    }

    // If only filename provided, consider common resource roots
    if (!containsSlash(path)) {
        cands.push_back("Resources/Shaders/" + path);
        cands.push_back("../Resources/Shaders/" + path);
        cands.push_back("../../Resources/Shaders/" + path);
    }

    return cands;
}

static bool loadFileToString(const std::vector<std::string>& candidates, std::string& out, std::string& tried) {
    for (const auto& p : candidates) {
        tried += "  - " + p + "\n";
        std::ifstream f(p, std::ios::in | std::ios::binary);
        if (!f.is_open()) continue;
        std::stringstream ss;
        ss << f.rdbuf();
        out = ss.str();
        return true;
    }
    return false;
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    // 1. Retrieve the vertex/fragment source code from filePath with fallbacks
    std::string vertexCode;
    std::string fragmentCode;

    std::string triedV, triedF;
    auto vCands = buildCandidates(vertexPath ? std::string(vertexPath) : std::string());
    auto fCands = buildCandidates(fragmentPath ? std::string(fragmentPath) : std::string());

    bool vOk = loadFileToString(vCands, vertexCode, triedV);
    bool fOk = loadFileToString(fCands, fragmentCode, triedF);

    if (!vOk || !fOk || vertexCode.empty() || fragmentCode.empty()) {
        std::stringstream err;
        err << "ERROR::SHADER::FILE_NOT_FOUND_OR_EMPTY\n";
        if (!vOk || vertexCode.empty()) {
            err << "Vertex shader search tried:\n" << triedV;
        }
        if (!fOk || fragmentCode.empty()) {
            err << "Fragment shader search tried:\n" << triedF;
        }
        throw std::runtime_error(err.str());
    }
    
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    
    // 2. Compile shaders
    unsigned int vertex, fragment;
    
    // Vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    
    // Fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    
    // Shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    
    // Delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() const { 
    glUseProgram(ID); 
}

void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}

void Shader::setInt(const std::string &name, int value) const { 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setFloat(const std::string &name, float value) const { 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const { 
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const { 
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const { 
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(unsigned int shader, const std::string &type) {
    int success;
    char infoLog[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" 
                      << infoLog << "\n -- --------------------------------------------------- -- " 
                      << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" 
                      << infoLog << "\n -- --------------------------------------------------- -- " 
                      << std::endl;
        }
    }
}