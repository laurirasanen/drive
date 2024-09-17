#include "Engine.h"
#include "Log.h"

#include <exception>

int main(int /*argc*/, char** /*argv*/)
{
#if NDEBUG
    drive::Log::SetLogLevel(drive::LogLevel::Warning);
#else
    drive::Log::SetLogLevel(drive::LogLevel::Debug);
#endif

    try
    {
        drive::Engine engine;
    }
    catch (std::exception& ex)
    {
        LOG_EXCEPTION("Unhandled exception: {}", ex.what());
        drive::Log::Flush();
        return -1;
    }

    return 0;
}
