#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <ostream>
#include <regex>
#include <vector>

#include "Log.h"

int main(int argc, char** argv)
{
    drive::Log::SetLogLevel(drive::LogLevel::Debug);

    try
    {
        if (argc != 3)
        {
            LOG_ERROR("Invalid number of arguments");
            LOG_ERROR("Usage: ShaderCompiler <src_dir> <out_dir>");
            return -1;
        }

        const auto srcDir = std::filesystem::path(argv[1]);
        const auto outDir = std::filesystem::path(argv[2]);

        if (std::filesystem::exists(outDir))
        {
            LOG_INFO("Removing existing target directory '{}'", outDir.string());
            std::filesystem::remove_all(outDir);
        }

        LOG_INFO("Creating target directory '{}'", outDir.string());
        std::filesystem::create_directory(outDir);

        const auto regex       = std::regex(".*\\.(frag|vert)");
        auto       shaderFiles = std::vector<std::filesystem::path>();

        LOG_INFO("Shaders in '{}':", srcDir.string());
        for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(srcDir))
        {
            if (!std::filesystem::is_regular_file(dir_entry))
            {
                continue;
            }

            const auto filepath = dir_entry.path();
            const auto filename = filepath.filename().string();
            if (!std::regex_match(filename, regex))
            {
                continue;
            }

            LOG_INFO("  {}", filename);
            shaderFiles.push_back(filepath);
        }

        const auto now       = std::chrono::system_clock::now();
        const auto fileStart = std::format(
            "// Generated by ShaderCompiler\n"
            "// {:%Y-%m-%d %H:%M:%OS}\n"
            "\n"
            "#pragma once\n",
            now
        );
        const auto namespaceStart = "namespace drive\n{";
        const auto namespaceEnd   = "}; // namespace drive";

        const auto    metaPath = std::filesystem::path(outDir).append("Shaders.h");
        std::ofstream metaFile(metaPath, std::ios::app);
        metaFile << fileStart << std::endl;

        for (auto f : shaderFiles)
        {
            const auto shaderName = f.filename().string();
            LOG_INFO("Compiling '{}'", shaderName);
            const auto spvPath = std::filesystem::path(outDir).append(shaderName).concat(".spv");
            const auto headerPath =
                std::filesystem::path(outDir).append(shaderName).concat(".spv.h");

            // Compile to SPV
            // TODO: use libshaderc instead
            const auto glslc =
                std::format("glslc {} -o {} -I {}", f.string(), spvPath.string(), srcDir.string());
            const auto ret = system(glslc.c_str());
            if (ret != 0)
            {
                LOG_ERROR("glslc failed: {}", ret);
                return -1;
            }
            LOG_INFO("Wrote SPV '{}'", spvPath.string());

            auto codeName = std::filesystem::path(f).filename().string();
            codeName.replace(codeName.find("."), 1, "_");

            auto spvFile = std::ifstream(spvPath, std::ios::binary);
            if (!spvFile.is_open())
            {
                LOG_ERROR("Failed to open SPV file '{}'", spvPath.string());
                return -1;
            }
            const auto spvLength = std::filesystem::file_size(spvPath);
            auto       spvBytes  = std::vector<char>(spvLength);
            spvFile.read(spvBytes.data(), static_cast<std::streamsize>(spvLength));

            // Create header from SPV bytecode
            std::ofstream headerFile(headerPath, std::ios::app);
            headerFile << "// Shader: " << shaderName << std::endl;
            headerFile << fileStart << std::endl;
            headerFile << namespaceStart << std::endl;
            headerFile << "constexpr static unsigned char " << codeName << "_spv[] = {"
                       << std::endl;

            const char hexDigits[] = "0123456789ABCDEF";
            const auto bytesPerRow = 8;
            for (unsigned int i = 0; i < spvLength; i++)
            {
                if (i % bytesPerRow == 0)
                {
                    headerFile << "    ";
                }
                const unsigned char c      = static_cast<unsigned char>(spvBytes[i]);
                const unsigned int  first  = c >> 4;
                const unsigned int  second = c & 15;
                headerFile << std::format("0x{}{}", hexDigits[first], hexDigits[second]);
                if (i < spvLength - 1)
                {
                    headerFile << ", ";
                }
                if ((i + 1) % bytesPerRow == 0 || i == spvLength - 1)
                {
                    headerFile << std::endl;
                }
            }
            headerFile << "};" << std::endl;
            headerFile << "constexpr static unsigned int " << codeName
                       << "_spv_len = " << std::format("{}", spvLength) << ";" << std::endl;
            headerFile << namespaceEnd << std::endl;

            LOG_INFO("Wrote header '{}'\n", headerPath.string());

            // Add to single include file
            metaFile << "#include " << "\"" << headerPath.filename().string() << "\"" << std::endl;
        }

        LOG_INFO("Wrote single include '{}'", metaPath.string());
    }
    catch (std::exception& ex)
    {
        LOG_EXCEPTION("Unhandled exception: {}\n{}", ex.what());
        drive::Log::Flush();
        return -1;
    }

    return 0;
}
