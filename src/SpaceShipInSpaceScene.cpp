#include <BadgerEngine/Camera.h>
#include <BadgerEngine/Geometry/Primitives.h>
#include <BadgerEngine/Import/TinyGLTFImporter.h>
#include <BadgerEngine/Model/Model.h>
#include <BadgerEngine/Renderer.h>
#include <BadgerEngine/VertexObject.h>
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
#include <background.frag.spv.h>
#include <background.vert.spv.h>
#include <chrono>
#include <deque>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ranges>
#include <ship.bin.h>
#include <ship.gltf.h>
#include <thread>
#include <unlit_shadow.frag.spv.h>
#include <vert_uniform.vert.spv.h>

using namespace BadgerEngine;

std::optional<MaterialColorChannel> baseColorFromImportedMesh(const Import::Mesh& mesh)
{
    if (!mesh.material()->baseColorMaps.empty()) {
        return mesh.material()->baseColorMaps.front();
    } else if (const auto baseColor = mesh.material()->baseColor) {
        return *baseColor;
    } else {
        return std::nullopt;
    }
}

std::optional<MaterialColorChannel> ambientOclusionMapFromImportedMesh(const Import::Mesh& mesh)
{
    if (!mesh.material()->ambientOcclusionMaps.empty()) {
        return mesh.material()->ambientOcclusionMaps.front();
    } else if (!mesh.material()->lightmapMaps.empty()) {
        return mesh.material()->lightmapMaps.front();
    } else {
        return std::nullopt;
    }
}

std::optional<MaterialColorChannel> normalMapFromImportedMesh(const Import::Mesh& mesh)
{
    if (!mesh.material()->normalsMaps.empty()) {
        return mesh.material()->normalsMaps.front();
    } else {
        return std::nullopt;
    }
}

Model modelFromImported(const Import::Mesh& mesh, bool castShadow)
{
    const auto baseColor = baseColorFromImportedMesh(mesh);
    const auto ambientOclusionMap = ambientOclusionMapFromImportedMesh(mesh);
    const auto normalMap = normalMapFromImportedMesh(mesh);

    return Model(
        mesh.mesh(),
        BSDFMaterial {
            .baseColor = baseColor.value(),
            .normalMap = normalMap.value_or(glm::vec4(0.50196, 0.50196, 1, 1)),
            .ambientOclusion = ambientOclusionMap.value_or(glm::vec4(1.)),
            .metallness = 0.f,
            .roughness = 0.5f,
            .indexOfRefration = 1.45f,
            .castShadow = castShadow,
        },
        PolygonMode::Fill);
}

int main(int argc, const char** argv)
{
    const auto window = std::make_shared<SDL2Window>("Space ship in space scene", 1400, 800);
    window->setCursorVisible(false).transform_error(AsCritical());
    const auto camera = std::make_shared<PerspectiveCamera>();

    Renderer renderer(window, camera, {});
    TextureLoader textureLoader;

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

    const Shared<Geometry::Mesh> backgroundMesh = Geometry::Mesh::create(
        Geometry::Topology::TriangleList,
        std::vector<Geometry::Vertex> {
            { .position = { -1.f, 1.f, 0.9999 } },
            { .position = { 1.f, 1.f, 0.9999 } },
            { .position = { 1.f, -1.f, 0.9999 } },
            { .position = { -1.f, -1.f, 0.9999 } },
        },
        { 2, 1, 0, 0, 3, 2 });

    renderer.addObject(BadgerEngine::Model(
        backgroundMesh,
        BadgerEngine::CustomMaterial {
            .vert = { background_vert.begin(), background_vert.end() },
            .frag = { background_frag.begin(), background_frag.end() },
        },
        BadgerEngine::PolygonMode::Fill));

    const Shared<Geometry::Mesh> planeMesh = Geometry::Mesh::create(
        Geometry::Topology::TriangleList,
        std::vector<Geometry::Vertex> {
            { .position = { 1.f, 1.f, 0 }, .normal = { 0, 0, 1.f }, .uv = { 0.0f, 0.0f } },
            { .position = { -1.f, 1.f, 0 }, .normal = { 0, 0, 1.f }, .uv = { 1.0f, 0.0f } },
            { .position = { -1.f, -1.f, 0 }, .normal = { 0, 0, 1.f }, .uv = { 1.0f, 1.0f } },
            { .position = { 1.f, -1.f, 0 }, .normal = { 0, 0, 1.f }, .uv = { 0.0f, 1.0f } },
        },
        std::vector<std::uint32_t> { 0, 1, 2,
            2, 3, 0 });

    renderer.addObject(
                BadgerEngine::Model(
                    planeMesh,
                    BadgerEngine::ShadowMapMaterial {
                        .vert = { vert_uniform_vert.begin(), vert_uniform_vert.end() },
                        .frag = { unlit_shadow_frag.begin(), unlit_shadow_frag.end() },
                    },
                    BadgerEngine::PolygonMode::Fill),
                RenderingOptions { .backfaceCulling = false })
        .setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(-1, 5, 0)));

    auto shipMeshes
        = Import::TinyGLTFImporter()
              .parse(textureLoader, ship_gltf, { { "ship.bin", ship_bin } }, "gltf")
              .transform_error(BadgerEngine::AsCritical())
              .value()
              .meshes()
        | std::ranges::to<std::deque>();

    renderer.directionalLight().setShadowFocus(glm::vec3(0, 0, -15.f));
    renderer.directionalLight().setShadowNear(-10);
    renderer.directionalLight().setShadowFar(10);
    renderer.directionalLight().setShadowCameraScale(250);
    renderer.directionalLight().setShadowBias(0.005);
    renderer.directionalLight().setIntensity(0.5);

    std::chrono::time_point prev = std::chrono::high_resolution_clock::now();

    const auto desiredFps = 60;
    float lightAngle = 0;

    while (!(window->shouldClose() || window->pressed(BadgerEngine::InputProvider::Key::Escape))) {
        const auto now = std::chrono::high_resolution_clock::now();
        const auto dt = now - std::exchange(prev, now);

        if (!shipMeshes.empty()) {
            renderer.addObject(modelFromImported(*shipMeshes.front(), true))
                .setScale(glm::scale(glm::mat4(1.), { 0.5, 0.5, 0.5 }))
                .setRotation(glm::rotate(glm::mat4(1.), -std::numbers::pi_v<float> / 2, glm::vec3(0, 1, 0)))
                .setTranslation(glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -15.f)));

            shipMeshes.pop_front();
        }

        {
            const float dtSec = std::chrono::duration_cast<std::chrono::milliseconds>(dt).count() / 1000.;
            if (window->pressed(BadgerEngine::InputProvider::Key::Minus)) {
                lightAngle -= 2 * dtSec;
            } else if (window->pressed(BadgerEngine::InputProvider::Key::Equal)) {
                lightAngle += 2 * dtSec;
            } else {
                lightAngle += 0.1 * dtSec;
            }

            const auto x = std::cos(lightAngle);
            const auto y = std::sin(lightAngle);

            renderer.directionalLight().setDirection(glm::rotate(glm::mat4(1.), std::numbers::pi_v<float> / 4, glm::vec3(0, 1, 0)) * glm::vec4(x, y, 0, 0));
        }

        camera->processInput(*window, dt);
        renderer.applyPresentation().transform_error(AsCritical());

        const auto desiredDt = std::chrono::milliseconds(1000 / desiredFps);
        if (dt < desiredDt) {
            using std::chrono::operator""ms;
            std::this_thread::sleep_for(desiredDt - dt);
        }
    }

    return 0;
}
