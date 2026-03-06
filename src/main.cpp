

#include <BadgerEngine/Camera.h>
#include <BadgerEngine/Geometry/Mesh.h>
#include <BadgerEngine/Geometry/ObjMesh.h>
#include <BadgerEngine/Geometry/Primitives.h>
#include <BadgerEngine/PointLight.h>
#include <BadgerEngine/Tools/Model.h>
#include <BadgerEngine/Utils/Fs.h>
#include <BadgerEngine/Window.h>
#include <BadgerEngine/Windows/GLFWWindow.h>
#include <BadgerEngine/Windows/SDL2Window.h>
#include <BadgerEngine/renderer.h>
#include <BadgerEngine/vertexobject.h>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <thread>

using namespace BadgerEngine;
using namespace BadgerEngine::Geometry;

glm::vec2 cursorPosition(SDL2Window& window)
{
    const auto pos = window.mousePosition();
    const auto center = window.size() / 2.f;
    static glm::vec2 r = { 0, 0 };
    r += (pos - center);
    window.setCursorVisible(false);
    window.setMousePosition(center);
    return r;
}

void updateRotation(SDL2Window& window, PerspectiveCamera& camera)
{
    const auto cpos = cursorPosition(window);
    const auto size = window.size();

    const auto tx = std::fmod(cpos.x / size.x, std::numbers::pi_v<float> * 4);

    const auto xq = glm::angleAxis(tx, glm::vec3(0, 1, 0));
    const auto yq = glm::angleAxis(-cpos.y / size.y, glm::vec3(1, 0, 0) * xq);

    camera.setRotation(xq * yq);
}

void updateTranslation(SDL2Window& window, PerspectiveCamera& camera)
{
    glm::vec3 direction = {
        0,
        0,
        0
    };
    if (window.isPressed(Window::Key::A)) {
        direction.x = 1;
    } else if (window.isPressed(Window::Key::D)) {
        direction.x = -1;
    }

    if (window.isPressed(Window::Key::W)) {
        direction.z = 1;
    } else if (window.isPressed(Window::Key::S)) {
        direction.z = -1;
    }

    // std::cout << "direction: " << direction.x << ", " << direction.y << ", " << direction.z << std::endl;

    float velocity = 0.1;

    camera.addTranslation((direction * velocity) * camera.rotation());
}

int main(int argc, const char** argv)
{
    assert(argc > 0);
    const auto executableDir = std::filesystem::path(argv[0]).parent_path();

    // const auto window = std::make_shared<GLFWWindow>("test_3d_game", 1000, 600);
    const auto window = std::make_shared<SDL2Window>("test_3d_game", 1000, 600);

    Renderer renderer(window, executableDir / "fonts/ZCOOL.ttf");

    const Mesh planeMesh = Mesh(
        Topology::TriangleList,
        { { { 1.f, 1.f, 0 }, { 0, 0, 1.f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { -1.f, 1.f, 0 }, { 0, 0, 1.f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
            { { -1.f, -1.f, 0 }, { 0, 0, 1.f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
            { { 1.f, -1.f, 0 }, { 0, 0, 1.f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } } },
        { 0, 1, 2,
            2, 3, 0 });

    auto plate = renderer.addObject(
        Model(planeMesh,
            Fs::readBinary(executableDir / "shaders/vert_uniform.vert.spv").transform_error(BadgerEngine::handleAsCritical<>).value(),
            Fs::readBinary(executableDir / "shaders/plane.frag.spv").transform_error(BadgerEngine::handleAsCritical<>).value()));

    auto cube = renderer.addObject(
        Model(Mesh::fromObjMesh(ObjMesh::load(executableDir / "models/untitled.obj").transform_error(BadgerEngine::handleAsCritical<>).value(), glm::vec3(0, 0, 0)),
            Fs::readBinary(executableDir / "shaders/vert_uniform.vert.spv").transform_error(BadgerEngine::handleAsCritical<>).value(),
            Fs::readBinary(executableDir / "shaders/lit.frag.spv").transform_error(BadgerEngine::handleAsCritical<>).value()));
    cube->setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(10.f, 0, 0)));

    auto monkey = renderer.addObject(
        Model(Mesh::fromObjMesh(ObjMesh::load(executableDir / "models/monkey.obj").transform_error(handleAsCritical<>).value(), glm::vec3(0, 0, 0)),
            Fs::readBinary(executableDir / "shaders/vert_uniform.vert.spv").transform_error(BadgerEngine::handleAsCritical<>).value(),
            Fs::readBinary(executableDir / "shaders/lit.frag.spv").transform_error(BadgerEngine::handleAsCritical<>).value()));
    monkey->setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(10.f, 5.f, 0)));

    const auto blueLight = renderer.addPointLight({ 1.f, 1.f, 1.f }, { 0.f, 0.f, 1.f }, 50.f);
    const auto yellowLight = renderer.addPointLight({ 1.f, 1.f, 1.f }, { 1.f, 1.f, 0.f }, 50.f);
    const auto blueLightObj = renderer.addObject(
        Model(Mesh::fromObjMesh(ObjMesh::load(executableDir / "models/untitled.obj").transform_error(BadgerEngine::handleAsCritical<>).value(), glm::vec3(0, 0, 0)),
            Fs::readBinary(executableDir / "shaders/vert_uniform.vert.spv").transform_error(BadgerEngine::handleAsCritical<>).value(),
            Fs::readBinary(executableDir / "shaders/plane.frag.spv").transform_error(BadgerEngine::handleAsCritical<>).value()));
    const auto yellowLightObj = renderer.addObject(
        Model(Mesh::fromObjMesh(ObjMesh::load(executableDir / "models/untitled.obj").transform_error(BadgerEngine::handleAsCritical<>).value(), glm::vec3(0, 0, 0)),
            Fs::readBinary(executableDir / "shaders/vert_uniform.vert.spv").transform_error(BadgerEngine::handleAsCritical<>).value(),
            Fs::readBinary(executableDir / "shaders/plane.frag.spv").transform_error(BadgerEngine::handleAsCritical<>).value()));

    std::chrono::time_point begin
        = std::chrono::high_resolution_clock::now();
    while (!window->shouldClose()) {
        const auto dur = std::chrono::high_resolution_clock::now() - begin;
        const float durSec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() / 1000.;

        {
            {
                const auto x = std::cos(durSec);
                const auto y = std::sin(durSec);
                const auto pos = glm::vec3(5.f - x * 4.f, x * 4.f, y * 8.f);
                blueLightObj->setTranslation(glm::translate(glm::mat4(1.f), pos));
                blueLight->position = pos;
            }
            {
                const auto x = std::cos(durSec / 2);
                const auto y = std::sin(durSec / 2);
                const auto pos = glm::vec3(5.f + x * 8.f, x * 8.f, -y * 4.f);
                yellowLightObj->setTranslation(glm::translate(glm::mat4(1.f), pos));
                yellowLight->position = pos;
            }
        }

        // if (window->isPressed(Window::Key::A)) {
        //     std::cout << "durSec: " << durSec << std::endl;
        // }

        // renderer.camera()->setTranslation(glm::vec3(0., 0., -durSec / 10.));
        updateRotation(*window, *renderer.camera());
        updateTranslation(*window, *renderer.camera());

        renderer.applyPresentation();
        using std::chrono::operator""ms;
        std::this_thread::sleep_for(4ms);
    }
    return 0;
}
