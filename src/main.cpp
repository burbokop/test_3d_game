

#include "lit.frag.spv.h"
#include "prism.frag.spv.h"
#include "unlit.frag.spv.h"
#include "vert_uniform.vert.spv.h"
#include <BadgerEngine/Camera.h>
#include <BadgerEngine/Geometry/Mesh.h>
#include <BadgerEngine/Geometry/ObjMesh.h>
#include <BadgerEngine/Geometry/Primitives.h>
#include <BadgerEngine/Import/AssimpImporter.h>
#include <BadgerEngine/Import/Importer.h>
#include <BadgerEngine/Import/TinyGLTFImporter.h>
#include <BadgerEngine/Model/Model.h>
#include <BadgerEngine/PointLight.h>
#include <BadgerEngine/Renderer.h>
#include <BadgerEngine/Utils/Fs.h>
#include <BadgerEngine/VertexObject.h>
#include <BadgerEngine/Window.h>
#include <BadgerEngine/Windows/GLFWWindow.h>
#include <BadgerEngine/Windows/SDL2Window.h>
#include <Body.png.h>
#include <Body_AO.png.h>
#include <Body_NM2.png.h>
#include <Guns.png.h>
#include <Guns_AO.png.h>
#include <Metal_001_AO-Metal_001_roughness.png.h>
#include <Metal_001_normal.png.h>
#include <Metal_001_roughness.png.h>
#include <MissileRack.png.h>
#include <MissileRack_AO.png.h>
#include <ZCOOL.ttf.h>
#include <background.frag.spv.h>
#include <background.vert.spv.h>
#include <chrono>
#include <deque>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <monkey.obj.h>
#include <ranges>
#include <ship.bin.h>
#include <ship.gltf.h>
#include <thread>
#include <unlit_shadow.frag.spv.h>
#include <untitled.obj.h>

using namespace BadgerEngine::Geometry;
namespace Import = BadgerEngine::Import;
using AsCritical = BadgerEngine::AsCritical<>;
template<typename T>
using Shared = BadgerEngine::Shared<T>;

using Win = BadgerEngine::SDL2Window;

// glm::vec2 cursorPosition(Win& window, float dt)
// {
//     const auto pos = window.mousePosition();
//     const auto center = window.size() / 2.f;
//     static glm::vec2 r = { 0, 0 };
//     r += (pos - center) * dt;

//     // window.setCursorVisible(false);
//     window.setMousePosition(center);
//     return r;
// }

// void updateRotation(Win& window, PerspectiveCamera& camera, float dt)
// {
//     const auto cpos = cursorPosition(window, dt);
//     const auto size = window.size();

//     const auto tx = std::fmod(cpos.x / size.x, std::numbers::pi_v<float> * 4);

//     const auto xq = glm::angleAxis(tx, glm::vec3(0, 1, 0));
//     const auto yq = glm::angleAxis(-cpos.y / size.y, glm::vec3(1, 0, 0) * xq);

//     camera.setRotation(xq * yq);
// }

/**
 * @brief vectorAlignmentMatrix - see https://stackoverflow.com/questions/34366655/glm-make-rotation-matrix-from-vec3
 * @param target
 * @param soruce
 * @return
 */
glm::mat4 vectorAlignmentMatrix(glm::vec3 target, glm::vec3 source)
{
    const glm::vec3 a = target;
    const glm::vec3 b = source;
    glm::vec3 v = glm::cross(b, a);
    float angle = acos(glm::dot(b, a) / (glm::length(b) * glm::length(a)));
    return glm::rotate(glm::mat4(1.), angle, v);
}

// void updateTranslation(Win& window, PerspectiveCamera& camera)
// {
//     glm::vec3 direction = {
//         0,
//         0,
//         0
//     };
//     if (window.isPressed(Window::Key::A)) {
//         direction.x = 1;
//     } else if (window.isPressed(Window::Key::D)) {
//         direction.x = -1;
//     }

//     if (window.isPressed(Window::Key::W)) {
//         direction.z = 1;
//     } else if (window.isPressed(Window::Key::S)) {
//         direction.z = -1;
//     }

//     // std::cout << "direction: " << direction.x << ", " << direction.y << ", " << direction.z << std::endl;

//     float velocity = 0.1;

//     camera.addPosition((direction * velocity) * camera.rotation());
// }

std::optional<BadgerEngine::MaterialColorChannel> baseColorFromImportedMesh(const Import::Mesh& mesh)
{
    if (!mesh.material()->baseColorMaps.empty()) {
        return mesh.material()->baseColorMaps.front();
    } else if (const auto baseColor = mesh.material()->baseColor) {
        return *baseColor;
    } else {
        return std::nullopt;
    }
}

std::optional<BadgerEngine::MaterialColorChannel> ambientOclusionMapFromImportedMesh(const Import::Mesh& mesh)
{
    if (!mesh.material()->ambientOcclusionMaps.empty()) {
        return mesh.material()->ambientOcclusionMaps.front();
    } else if (!mesh.material()->lightmapMaps.empty()) {
        return mesh.material()->lightmapMaps.front();
    } else {
        return std::nullopt;
    }
}

std::optional<BadgerEngine::MaterialColorChannel> normalMapFromImportedMesh(const Import::Mesh& mesh)
{
    if (!mesh.material()->normalsMaps.empty()) {
        return mesh.material()->normalsMaps.front();
    } else {
        return std::nullopt;
    }
}

BadgerEngine::Model modelFromImported(const Import::Mesh& mesh, bool castShadow)
{
    const auto baseColor = baseColorFromImportedMesh(mesh);
    const auto ambientOclusionMap = ambientOclusionMapFromImportedMesh(mesh);
    const auto normalMap = normalMapFromImportedMesh(mesh);

    if (baseColor) {
        return BadgerEngine::Model(
            mesh.mesh(),
            BadgerEngine::BSDFMaterial {
                .baseColor = *baseColor,
                .normalMap = normalMap.value_or(glm::vec4(0.50196, 0.50196, 1, 1)),
                .ambientOclusion = ambientOclusionMap.value_or(glm::vec4(1.)),
                .metallness = 0.f,
                .roughness = 0.5f,
                .indexOfRefration = 1.45f,
                .castShadow = castShadow,
            },
            BadgerEngine::PolygonMode::Fill);
    } else {
        return BadgerEngine::Model(
            mesh.mesh(),
            BadgerEngine::CustomMaterial {
                .textures = {},
                .vert = { vert_uniform_vert.begin(), vert_uniform_vert.end() },
                .frag = { prism_frag.begin(), prism_frag.end() },
            },
            BadgerEngine::PolygonMode::Fill);
    }
}

BadgerEngine::Model bsdfModelFromMesh(const Shared<Mesh>& mesh, bool castShadow)
{
    return BadgerEngine::Model(
        mesh,
        BadgerEngine::BSDFMaterial {
            .baseColor = glm::vec4(0.5, 0.5, 0.5, 1),
            .normalMap = glm::vec4(0.50196, 0.50196, 1, 1),
            .ambientOclusion = glm::vec4(1.),
            .metallness = 0.f,
            .roughness = 0.5f,
            .indexOfRefration = 1.45f,
            .castShadow = castShadow,
        },
        BadgerEngine::PolygonMode::Fill);
}

int main(int argc, const char** argv)
{
    assert(argc > 0);
    const auto executableDir = std::filesystem::path(argv[0]).parent_path();

    const auto window = std::make_shared<Win>("test_3d_game", 1400, 800);
    window->setCursorVisible(false).transform_error(AsCritical());
    const auto camera = std::make_shared<BadgerEngine::PerspectiveCamera>();

    // const auto directionalLightCamera = std::make_shared<BadgerEngine::DirectionBasedOrthographicCamera>();

    BadgerEngine::Renderer renderer(window, camera, ZCOOL_ttf);
    // BadgerEngine::Renderer renderer(window, directionalLightCamera, ZCOOL_ttf);

    BadgerEngine::TextureLoader textureLoader;

    const Shared<Mesh> backgroundMesh = Mesh::create(
        Topology::TriangleList,
        std::vector<Vertex> {
            { .position = { -1.f, 1.f, 0.9999 }, .normal = { 0, 0, 1.f }, .tangent = { 1, 0, 0 }, .bitangent = { 0, 1, 0 }, .color = { 0.0f, 1.0f, 0.0f }, .uv = { 1.0f, 0.0f } },
            { .position = { 1.f, 1.f, 0.9999 }, .normal = { 0, 0, 1.f }, .tangent = { 1, 0, 0 }, .bitangent = { 0, 1, 0 }, .color = { 1.0f, 0.0f, 0.0f }, .uv = { 0.0f, 0.0f } },
            { .position = { 1.f, -1.f, 0.9999 }, .normal = { 0, 0, 1.f }, .tangent = { 1, 0, 0 }, .bitangent = { 0, 1, 0 }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 0.0f, 1.0f } },
            { .position = { -1.f, -1.f, 0.9999 }, .normal = { 0, 0, 1.f }, .tangent = { 1, 0, 0 }, .bitangent = { 0, 1, 0 }, .color = { 0.0f, 0.0f, 1.0f }, .uv = { 1.0f, 1.0f } },
        },
        std::vector<std::uint32_t> { 2, 1, 0, 0, 3, 2 });

    renderer.addObject(BadgerEngine::Model(
        backgroundMesh,
        BadgerEngine::CustomMaterial {
            .vert = { background_vert.begin(), background_vert.end() },
            .frag = { background_frag.begin(), background_frag.end() },
        },
        BadgerEngine::PolygonMode::Fill));

    textureLoader.parse({
                            { "../textures/Body.png", Body_png },
                            { "../textures/Body_AO.png", Body_AO_png },
                            { "../textures/Body_NM2.png", Body_NM2_png },
                            { "../textures/Guns.png", Guns_png },
                            { "../textures/Guns_AO.png", Guns_AO_png },
                            { "../textures/MissileRack.png", MissileRack_png },
                            { "../textures/MissileRack_AO.png", MissileRack_AO_png },
                            { "../textures/Metal_001_normal.png", Metal_001_normal_png },
                            // R channel - AO, G channel - Roughness, B channel - Metallness
                            { "../textures/Metal_001_AO-Metal_001_roughness.png", Metal_001_AO_minus_Metal_001_roughness_png },
                        })
        .transform_error(AsCritical());

    // window->drawTexture(textureLoader.texture("Body.png").value());

    // sleep(10);
    // return 0;

    const Shared<Mesh> planeMesh = Mesh::create(
        Topology::TriangleList,
        std::vector<Vertex> {
            { .position = { 1.f, 1.f, 0 }, .normal = { 0, 0, 1.f }, .tangent = { 1, 0, 0 }, .bitangent = { 0, 1, 0 }, .color = { 1.0f, 0.0f, 0.0f }, .uv = { 0.0f, 0.0f } },
            { .position = { -1.f, 1.f, 0 }, .normal = { 0, 0, 1.f }, .tangent = { 1, 0, 0 }, .bitangent = { 0, 1, 0 }, .color = { 0.0f, 1.0f, 0.0f }, .uv = { 1.0f, 0.0f } },
            { .position = { -1.f, -1.f, 0 }, .normal = { 0, 0, 1.f }, .tangent = { 1, 0, 0 }, .bitangent = { 0, 1, 0 }, .color = { 0.0f, 0.0f, 1.0f }, .uv = { 1.0f, 1.0f } },
            { .position = { 1.f, -1.f, 0 }, .normal = { 0, 0, 1.f }, .tangent = { 1, 0, 0 }, .bitangent = { 0, 1, 0 }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 0.0f, 1.0f } },
        },
        std::vector<std::uint32_t> { 0, 1, 2,
            2, 3, 0 });

    auto& plate = renderer.addObject(
        BadgerEngine::Model(
            planeMesh,
            BadgerEngine::CustomMaterial {
                .textures = {},
                .vert = { vert_uniform_vert.begin(), vert_uniform_vert.end() },
                .frag = { prism_frag.begin(), prism_frag.end() },
            },
            BadgerEngine::PolygonMode::Fill));

    {
        auto& plate1 = renderer.addObject(
                                   BadgerEngine::Model(
                                       planeMesh,
                                       BadgerEngine::CustomMaterial {
                                           .textures = { textureLoader.texture("../textures/Body.png").value() },
                                           .vert = { vert_uniform_vert.begin(), vert_uniform_vert.end() },
                                           .frag = { unlit_frag.begin(), unlit_frag.end() },
                                       },
                                       BadgerEngine::PolygonMode::Fill))
                           .setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(0, 0, 10.f)));
    }

    {
        auto& plate2 = renderer.addObject(
                                   BadgerEngine::Model(
                                       planeMesh,
                                       BadgerEngine::RecursiveMaterial {
                                           .vert = { vert_uniform_vert.begin(), vert_uniform_vert.end() },
                                           .frag = { unlit_frag.begin(), unlit_frag.end() },
                                       },
                                       BadgerEngine::PolygonMode::Fill))
                           .setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(0, 0, 20.f)));
    }

    {
        auto& plate3 = renderer.addObject(
                                   BadgerEngine::Model(
                                       planeMesh,
                                       BadgerEngine::ShadowMapMaterial {
                                           .vert = { vert_uniform_vert.begin(), vert_uniform_vert.end() },
                                           .frag = { unlit_shadow_frag.begin(), unlit_shadow_frag.end() },
                                       },
                                       BadgerEngine::PolygonMode::Fill))
                           .setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(-1, 5, 0)));
    }

    auto& cube = renderer.addObject(
        BadgerEngine::Model(
            Mesh::fromObjMesh(ObjMesh::parse(untitled_obj).transform_error(BadgerEngine::AsCritical()).value(), glm::vec3(0, 0, 0)),
            BadgerEngine::CustomMaterial {
                .textures = {},
                .vert = { vert_uniform_vert.begin(), vert_uniform_vert.end() },
                .frag = { lit_frag.begin(), lit_frag.end() },
            },
            BadgerEngine::PolygonMode::Fill));
    cube.setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(10.f, 0, 0)));

    auto& monkey = renderer.addObject(
        BadgerEngine::Model(
            Mesh::fromObjMesh(ObjMesh::parse(monkey_obj).transform_error(AsCritical()).value(), glm::vec3(0, 0, 0)),
            BadgerEngine::CustomMaterial {
                .textures = {},
                .vert = { vert_uniform_vert.begin(), vert_uniform_vert.end() },
                .frag = { lit_frag.begin(), lit_frag.end() },
            },
            BadgerEngine::PolygonMode::Fill));
    monkey.setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(10.f, 5.f, 0)));

    const auto blueLight = renderer.addPointLight({ 1.f, 1.f, 1.f }, { 0.f, 0.f, 1.f }, 50.f);
    const auto yellowLight = renderer.addPointLight({ 1.f, 1.f, 1.f }, { 1.f, 1.f, 0.f }, 50.f);
    auto& blueLightObj = renderer.addObject(
        BadgerEngine::Model(
            Mesh::fromObjMesh(ObjMesh::parse(untitled_obj).transform_error(BadgerEngine::AsCritical()).value(), glm::vec3(0, 0, 0)),
            BadgerEngine::CustomMaterial {
                .textures = {},
                .vert = { vert_uniform_vert.begin(), vert_uniform_vert.end() },
                .frag = { prism_frag.begin(), prism_frag.end() },
            },
            BadgerEngine::PolygonMode::Fill));

    auto& yellowLightObj = renderer.addObject(
        BadgerEngine::Model(
            Mesh::fromObjMesh(ObjMesh::parse(untitled_obj).transform_error(BadgerEngine::AsCritical()).value(), glm::vec3(0, 0, 0)),
            BadgerEngine::CustomMaterial {
                .textures = {},
                .vert = { vert_uniform_vert.begin(), vert_uniform_vert.end() },
                .frag = { prism_frag.begin(), prism_frag.end() },
            },
            BadgerEngine::PolygonMode::Fill));

    // {
    //     // const auto rossiModel = AssimpModel::load(textureLoader, executableDir / "models/Tachi_LP.fbx").transform_error(BadgerEngine::AsCritical()).value();
    //     const auto rossiModel = AssimpModel::load(textureLoader, executableDir / "models/RocinanteFinal.fbx").transform_error(BadgerEngine::AsCritical()).value();

    //     return 0;
    //     for (auto mesh : rossiModel.meshes()) {
    //         renderer.addObject(
    //                     Model(
    //                         mesh->mesh(),
    //                         mesh->material()->ambientOcclusionMaps,
    //                         { vert_uniform_vert.begin(), vert_uniform_vert.end() },
    //                         { lit_frag.begin(), lit_frag.end() }, PolygonMode::Fill))
    //             .setScale(glm::scale(glm::mat4(1.), { 0.001, 0.001, 0.001 }))
    //             .setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -5.f)));
    //     }
    // }

    auto& directionalLightVector = renderer.addObject(
        BadgerEngine::Model(
            Mesh::create(
                Topology::LineList,
                std::vector<Vertex> {
                    { .position = { 0, 0, 0 }, .normal = { -1, 0, 0 }, .color = { 1, 0, 0 }, .uv = { 0.0f, 0.0f } },
                    { .position = { 100, 0, 0 }, .normal = { 1, 0, 0 }, .color = { 0, 1, 0 }, .uv = { 1.0f, 0.0f } },
                },
                std::vector<std::uint32_t> { 0, 1 }),
            BadgerEngine::CustomMaterial {
                .textures = {},
                .vert = { vert_uniform_vert.begin(), vert_uniform_vert.end() },
                .frag = { prism_frag.begin(), prism_frag.end() },
            },
            BadgerEngine::PolygonMode::Line));

    // auto& directionalLightPlane = renderer.addObject(
    //     BadgerEngine::Model(
    //         Mesh::create(
    //             Topology::TriangleList,
    //             std::vector<Vertex> {
    //                 { .position = { 0, 1.f, 1.f }, .normal = { 1, 0, 0 }, .color = { 1.0f, 0.0f, 0.0f }, .uv = { 0.0f, 0.0f } },
    //                 { .position = { 0, -1.f, 1.f }, .normal = { 1, 0, 0 }, .color = { 0.0f, 1.0f, 0.0f }, .uv = { 1.0f, 0.0f } },
    //                 { .position = { 0, -1.f, -1.f }, .normal = { 1, 0, 0 }, .color = { 0.0f, 0.0f, 1.0f }, .uv = { 1.0f, 1.0f } },
    //                 { .position = { 0, 1.f, -1.f }, .normal = { 1, 0, 0 }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 0.0f, 1.0f } },
    //             },
    //             std::vector<std::uint32_t> { 0, 1, 2,
    //                 2, 3, 0 }),
    //         BadgerEngine::CustomMaterial {
    //             .textures = {},
    //             .vert = { vert_uniform_vert.begin(), vert_uniform_vert.end() },
    //             .frag = { prism_frag.begin(), prism_frag.end() },
    //         },
    //         BadgerEngine::PolygonMode::Line),
    //     BadgerEngine::RenderingOptions { .backfaceCulling = false });

    // Import::AssimpImporter importer;
    Import::TinyGLTFImporter importer;

    // auto _experimentalShipMeshes
    //     = Import::AssimpImporter()
    //           .load(textureLoader, executableDir / "models/ship.gltf")
    //           .transform_error(BadgerEngine::AsCritical())
    //           .value()
    //           .meshes()
    //     | std::ranges::to<std::deque>();

    auto experimentalShipMeshes
        = importer
              .parse(textureLoader, ship_gltf, { { "ship.bin", ship_bin } }, "gltf")
              .transform_error(BadgerEngine::AsCritical())
              .value()
              .meshes()
        | std::ranges::to<std::deque>();

    const Shared<Mesh> horisontalPlaneMesh = Mesh::create(
        Topology::TriangleList,
        std::vector<Vertex> {
            { .position = { 20.f, 0, 20.f }, .normal = { 0, 1.f, 0 }, .tangent = { 1, 0, 0 }, .bitangent = { 0, 0, 1 }, .color = { 1.0f, 0.0f, 0.0f }, .uv = { 0.0f, 0.0f } },
            { .position = { -20.f, 0, 20.f }, .normal = { 0, 1.f, 0 }, .tangent = { 1, 0, 0 }, .bitangent = { 0, 0, 1 }, .color = { 0.0f, 1.0f, 0.0f }, .uv = { 1.0f, 0.0f } },
            { .position = { -20.f, 0, -20.f }, .normal = { 0, 1.f, 0 }, .tangent = { 1, 0, 0 }, .bitangent = { 0, 0, 1 }, .color = { 0.0f, 0.0f, 1.0f }, .uv = { 1.0f, 1.0f } },
            { .position = { 20.f, 0, -20.f }, .normal = { 0, 1.f, 0 }, .tangent = { 1, 0, 0 }, .bitangent = { 0, 0, 1 }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 0.0f, 1.0f } },
        },
        std::vector<std::uint32_t> { 0, 1, 2,
            2, 3, 0 });

    renderer.addObject(bsdfModelFromMesh(horisontalPlaneMesh, false))
        .setScale(glm::scale(glm::mat4(1.), { 0.5, 0.5, 0.5 }))
        .setRotation(glm::rotate(glm::mat4(1.), -std::numbers::pi_v<float> / 2, glm::vec3(0, 1, 0)))
        .setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(0, 5.f, -15.f)));

    renderer.directionalLight().setShadowFocus(glm::vec3(0, 0, -15.f));
    renderer.directionalLight().setShadowNear(-10);
    renderer.directionalLight().setShadowFar(10);
    renderer.directionalLight().setShadowCameraScale(250);

    // std::abort();

    // auto experimentalShipMeshes = Mesh::loadByAssimp(executableDir / "models/ship.glb").transform_error(BadgerEngine::AsCritical()).value() | std::ranges::to<std::deque>();
    // auto experimentalShipMeshes = Mesh::loadByAssimp(executableDir / "models/ship.stl").transform_error(BadgerEngine::AsCritical()).value() | std::ranges::to<std::deque>();

    std::chrono::time_point begin
        = std::chrono::high_resolution_clock::now();

    std::chrono::time_point prev
        = std::chrono::high_resolution_clock::now();

    const auto desiredFps = 60;

    float lightAngle = 0;

    while (!(window->shouldClose() || window->pressed(BadgerEngine::InputProvider::Key::Escape))) {
        const auto now = std::chrono::high_resolution_clock::now();
        const auto elapsedFromBegin = now - begin;
        const auto dt = now - std::exchange(prev, now);

        if (!experimentalShipMeshes.empty()) {
            const auto mesh = experimentalShipMeshes.front();

            // const auto options = BadgerEngine::RenderingOptions { .displayNormals = mesh->name() == "Main Body" ? BadgerEngine::DisplayNormals::VertexNormals : BadgerEngine::DisplayNormals::NoNormals };
            const auto options = BadgerEngine::RenderingOptions {};

            // std::cout
            //     << "loading mesh: " << mesh->name() << " -> " << mesh->mesh()->vertices().size() << ", " << mesh->mesh()->indices().size() << ", " << int(options.displayNormals) << std::endl;

            // const auto options = BadgerEngine::RenderingOptions {};

            renderer.addObject(modelFromImported(*mesh, true), options)
                .setScale(glm::scale(glm::mat4(1.), { 0.5, 0.5, 0.5 }))
                .setRotation(glm::rotate(glm::mat4(1.), -std::numbers::pi_v<float> / 2, glm::vec3(0, 1, 0)))
                .setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -15.f)));

            // renderer.addObject(
            //             Model(
            //                 mesh->mesh(),
            //                 mesh->material()->baseColorMaps,
            //                 Fs::readBinary(executableDir / "shaders/vert_uniform_bloated.vert.spv").transform_error(BadgerEngine::AsCritical()).value(),
            //                 Fs::readBinary(executableDir / "shaders/shader.frag.spv").transform_error(BadgerEngine::AsCritical()).value(), PolygonMode::Line))
            //     .setScale(glm::scale(glm::mat4(1.), { 0.5, 0.5, 0.5 }))
            //     .setRotation(glm::rotate(glm::mat4(1.), -std::numbers::pi_v<float> / 2, glm::vec3(0, 1, 0)))
            //     .setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -20.f)));

            experimentalShipMeshes.pop_front();

            if (experimentalShipMeshes.empty()) {
                std::cout << "experimental ship meshes loading finished" << std::endl;
            }
        }

        const float dtSec = std::chrono::duration_cast<std::chrono::milliseconds>(dt).count() / 1000.;

        {
            const float durSec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedFromBegin).count() / 1000.;

            {
                const auto x = std::cos(durSec);
                const auto y = std::sin(durSec);
                const auto pos = glm::vec3(5.f - x * 4.f, x * 4.f, y * 8.f);
                blueLightObj.setTranslation(glm::translate(glm::mat4(1.f), pos));
                blueLight->position = pos;
            }
            {
                const auto x = std::cos(durSec / 2);
                const auto y = std::sin(durSec / 2);
                const auto pos = glm::vec3(5.f + x * 8.f, x * 8.f, -y * 4.f);
                yellowLightObj.setTranslation(glm::translate(glm::mat4(1.f), pos));
                yellowLight->position = pos;
            }
            {
                if (window->pressed(BadgerEngine::InputProvider::Key::Minus)) {
                    lightAngle -= 2 * dtSec;
                } else if (window->pressed(BadgerEngine::InputProvider::Key::Equal)) {
                    lightAngle += 2 * dtSec;
                }

                if (window->pressed(BadgerEngine::InputProvider::Key::_1)) {
                    renderer.setMode(0);
                } else if (window->pressed(BadgerEngine::InputProvider::Key::_2)) {
                    renderer.setMode(1);
                } else if (window->pressed(BadgerEngine::InputProvider::Key::_3)) {
                    renderer.setMode(2);
                } else if (window->pressed(BadgerEngine::InputProvider::Key::_4)) {
                    renderer.setMode(3);
                } else if (window->pressed(BadgerEngine::InputProvider::Key::_5)) {
                    renderer.setMode(4);
                } else if (window->pressed(BadgerEngine::InputProvider::Key::_6)) {
                    renderer.setMode(5);
                }

                const auto x = std::cos(lightAngle);
                const auto y = std::sin(lightAngle);

                renderer.directionalLight().setDirection(glm::rotate(glm::mat4(1.), std::numbers::pi_v<float> / 4, glm::vec3(0, 1, 0)) * glm::vec4(x, y, 0, 0));
                renderer.directionalLight().setIntensity(0.5);

                directionalLightVector
                    .setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -15.f)))
                    .setRotation(vectorAlignmentMatrix(-renderer.directionalLight().direction(), { 1, 0, 0 }));

                monkey.setTranslation(renderer.directionalLight().shadowCameraPosition());

                // directionalLightPlane
                //     .setTranslation({ 0, 0, -5 })
                //     .setRotation(vectorAlignmentMatrix(renderer.directionalLightVector(), { 1, 0, 0 }));

                // directionalLightCamera->setOrbit({ 0, 0, -5 }, renderer.directionalLightVector());
            }
        }

        // if (window->isPressed(Window::Key::A)) {
        //     std::cout << "durSec: " << durSec << std::endl;
        // }

        // renderer.camera()->setTranslation(glm::vec3(0., 0., -durSec / 10.));

        // updateRotation(*window, *renderer.camera(), dtSec);
        // updateTranslation(*window, *renderer.camera());

        camera->processInput(*window, dt);

        // processInput(*camera, *window, dtSec);
        // mouseCallback(*camera, window->mousePosition().x, window->mousePosition().y);
        // window->setMousePosition(window->size() / 2.f);

        renderer.applyPresentation().transform_error(AsCritical());

        const auto desiredDt = std::chrono::milliseconds(1000 / desiredFps);

        // std::cout << "fps: " << 1. / dtSec << std::endl;

        if (dt < desiredDt) {
            using std::chrono::operator""ms;
            std::this_thread::sleep_for(desiredDt - dt);
        }
    }
    return 0;
}
