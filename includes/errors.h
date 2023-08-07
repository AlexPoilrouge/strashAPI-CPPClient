#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <exception>

#include <nlohmann/json.hpp>


using json= nlohmann::json;

namespace StrashBot{
namespace API{


struct FilePathRelated : public std::exception {
    FilePathRelated(std::string path= {}) : given_path(path) {}
    FilePathRelated(std::exception(e), std::string path= {}) : std::exception(e), given_path(path) {}

    virtual const char* what() noexcept = 0;
protected:
    std::string given_path;
};


struct NonExistantConfigFileException : public FilePathRelated {
    inline NonExistantConfigFileException(std::string path={}) : FilePathRelated(path) {}

    const char* what() noexcept override {
        return (std::string("Cannot read the given ") +
                ((!this->given_path.empty())? (std::string("(")+this->given_path+") " ): std::string{}) +
                "path…").c_str();
    }
};


struct ConfigJSONParsingError : public FilePathRelated, public json::exception {
    inline ConfigJSONParsingError(std::string path={})
        : FilePathRelated(path), json::exception(501, "")
    {}
    inline ConfigJSONParsingError(const json::exception& e, std::string path={})
        : FilePathRelated(path), json::exception(e)
    {}

    const char* what() noexcept override {
        return (std::string("Failure while parsing Config json file") +
                ((!this->given_path.empty())? (std::string(" ")+this->given_path): std::string{}) +
                "…" + json::exception::what()
            ).c_str();
    }
};

struct BadReadConfigFileException : public FilePathRelated {
    inline BadReadConfigFileException(std::string path={}) : FilePathRelated(path) {}
    inline BadReadConfigFileException(std::exception(e), std::string path={}) : FilePathRelated(e, path) {}

    const char* what() noexcept override {
        return (std::string("Cannot read the given ") +
                ((!this->given_path.empty())? (std::string("(")+this->given_path+") " ): std::string{}) +
                "path…" + std::exception::what()
            ).c_str();
    }
};

struct ConnectTimeoutException : public std::exception {
    inline ConnectTimeoutException(const std::string& domain, unsigned int port)
        : connectBaseUrl(domain+" (port: "+std::to_string(port)+")")
    {}

    const char* what() noexcept {
        return std::string("Timeout on connect to "+connectBaseUrl+"…").c_str();
    }

protected:
    std::string connectBaseUrl;
};

struct BadTokenKeyFilePathForJWT : public FilePathRelated {
    inline BadTokenKeyFilePathForJWT(std::string path={}) : FilePathRelated(path) {}

    const char* what() noexcept override {
        return (std::string("Cannot read the given ") +
                ((!this->given_path.empty())? (std::string("(")+this->given_path+") " ): std::string{}) +
                "path…").c_str();
    }
};

struct TokenReadingError : public std::exception {
    TokenReadingError(std::exception e) : std::exception(e) {}

    const char* what() noexcept {
        return (std::string("Error while trying to setup token - ") + std::exception::what()).c_str();
    }
};

struct TokenGetSignatureError : public std::exception {
    TokenGetSignatureError(std::string endpointName, std::exception e) : endpoint(endpointName), std::exception(e) {}

    const char* what() noexcept {
        return (std::string("Failing fetching signature for endpoint's '") + this->endpoint + "' token... - " + std::exception::what()).c_str();
    }

protected:
    std::string endpoint;
};

}
}

#endif // ERROR_H
