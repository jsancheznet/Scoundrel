#include "camera.h"
#include "application.h"
#include "log.h"

extern application Application;

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
    Camera.Projection = glm::perspective(glm::radians(Camera.Fov), Camera.AspectRatio, Camera.Near, Camera.Far);

    Log(Info, "CreateCamera() - Created Camera, Position: %.2f %.2f, %.2f, Target: %.2f %.2f, %.2f",
	Camera.Position.x, Camera.Position.y, Camera.Position.z, Camera.Target.x, Camera.Target.y, Camera.Target.z);

    return Camera;
}
