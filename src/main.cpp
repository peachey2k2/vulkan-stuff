#include "core.hpp"


int main(int argc, char** argv){
    wmac::Engine engine;
    try {
        engine.run();
    } catch (const wmac::engine_fatal_exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
