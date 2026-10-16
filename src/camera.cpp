#include "camera.h"
#include "application.h"
#include "renderer.h"
#include "mouse.h"
#include "log.h"

extern application Application;
extern renderer Renderer;

camera CreateCamera()
{
    camera Camera;

    Camera.Position = vec3(0.0f, 0.0f, 1.0f);
    Camera.Target = vec3(0.0f);
    Camera.Up = vec3(0.0f, 1.0f, 0.0f);

    Camera.View = glm::lookAt(Camera.Position, Camera.Target, Camera.Up);

    Camera.Fov = 90.0f;
    Camera.Near = 0.01f;
    Camera.Far = 1000.0f;

    Camera.AspectRatio = (f32)Application.WindowWidth / (f32)Application.WindowHeight;
    Camera.Perspective = glm::perspective(glm::radians(Camera.Fov), Camera.AspectRatio, Camera.Near, Camera.Far);
    Camera.Orthographic = glm::ortho(0.0f, (f32)Application.WindowWidth, (f32)Application.WindowHeight, 0.0f);

    Log(Info, "CreateCamera() - Created Camera, Position: %.2f %.2f, %.2f, Target: %.2f %.2f, %.2f",
	Camera.Position.x, Camera.Position.y, Camera.Position.z, Camera.Target.x, Camera.Target.y, Camera.Target.z);

    return Camera;
}

void camera::Update()
{
    AspectRatio = (f32)Application.WindowWidth / (f32)Application.WindowHeight;

    View = glm::lookAt(Position, Target, Up);
    Perspective = glm::perspective(glm::radians(Fov), AspectRatio, Near, Far);
    Orthographic = glm::ortho(0.0f, (f32)Application.WindowWidth, (f32)Application.WindowHeight, 0.0f);

    Renderer.UpdateCameraBuffer(this);
}

vec3 ScreenToWorldRay(mouse *Mouse, renderer *Renderer, camera *Camera)
{
    // TODO(Jsanchez): Tomar nota de los pasos y subirlo a un gist

    // Step 1 - Mouse NDC Coords
    f32 X_NDC = (Mouse->X * 2.0f) / Renderer->ViewportWidth - 1.0f;
    f32 Y_NDC = 1.0f - (2.0f * Mouse->Y) / Renderer->ViewportHeight;
    f32 Z_NDC = 1.0f;
    glm::vec3 RayNDC = glm::vec3(X_NDC, Y_NDC, Z_NDC);

    // Step 2 - Homogeneous Clip Coords - Make Ray Z point forwards, Forward is Negative Z on OpenGL
    glm::vec4 RayClip = vec4(RayNDC.x, RayNDC.y, -1.0f, 1.0f);

    // Step 3 - Ray to Eye Space - Multiply by inverse of the projection matrix and Set Z & WW to mean Forwards, and not a point
    glm::vec4 RayEyeSpace = glm::inverse(Camera->Perspective) * RayClip;
    RayEyeSpace = glm::vec4(RayEyeSpace.x, RayEyeSpace.y, -1.0f, 0.0f);

    // Step 4 - World Coordinates - Go to world coordinates and normalize the vector to mean a direction
    glm::vec4 RayWorld4 = glm::inverse(Camera->View) *glm::vec4(RayEyeSpace.x, RayEyeSpace.y, -1.0f, 0.0f);
    glm::vec3 RayWorld  = glm::normalize(glm::vec3(RayWorld4));

    return RayWorld;
}

// Intersects a ray with any plane, given by a point on the plane and its normal (doesn't need to be normalized).
// Works from both sides of the plane, returns false if the ray is parallel to the plane or the plane is behind the ray.
b32 RayPlaneIntersection(vec3 RayOrigin, vec3 RayDirection, vec3 PlanePoint, vec3 PlaneNormal, vec3 *Hit)
{
    f32 Denominator = glm::dot(PlaneNormal, RayDirection);
    if(glm::abs(Denominator) < 0.0001f) return false;

    f32 T = glm::dot(PlanePoint - RayOrigin, PlaneNormal) / Denominator;
    if(T < 0.0f) return false;

    *Hit = RayOrigin + T * RayDirection;
    return true;
}
