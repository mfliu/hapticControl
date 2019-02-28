#include "graphics.h"

extern HapticData hapticsData;
extern ControlData controlData;
GraphicsData graphicsData;

void initDisplay(void)
{
  graphicsData.stereoMode = C_STEREO_DISABLED;
  graphicsData.fullscreen = false;
  graphicsData.mirroredDisplay = false;
  if (!glfwInit()) {
    cout << "Failed GLFW initialization" << endl;
    cSleepMs(1000);
    return;
  }
  
  glfwSetErrorCallback(errorCallback);

  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  int w = 0.8 * mode->height;
  int h = 0.5 * mode->height;
  int x = 0.5 * (mode->width-w);
  int y = 0.5 * (mode->height-h);
  
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  if (graphicsData.stereoMode == C_STEREO_ACTIVE) {
    glfwWindowHint(GLFW_STEREO, GL_TRUE);
  }
  else {
    glfwWindowHint(GLFW_STEREO, GL_FALSE);
  }
  
  graphicsData.width = w;
  graphicsData.height = h;
  graphicsData.xPos = x;
  graphicsData.yPos = y;
  graphicsData.swapInterval = 1;  

  graphicsData.window = glfwCreateWindow(w, h, "CHAI3D", NULL, NULL);
  if (!graphicsData.window) {
    cout << "Failed to create window" << endl;
    cSleepMs(1000);
    glfwTerminate();
    return;
  }
  
  glfwGetWindowSize(graphicsData.window, &graphicsData.width, &graphicsData.height);
  glfwSetWindowPos(graphicsData.window, graphicsData.xPos, graphicsData.yPos);
  glfwSetKeyCallback(graphicsData.window, keySelectCallback);
  glfwSetWindowSizeCallback(graphicsData.window, resizeWindowCallback);
  glfwMakeContextCurrent(graphicsData.window);
  glfwSwapInterval(graphicsData.swapInterval);

#ifdef GLEW_VERSION
  if(glewInit() != GLEW_OK) {
    cout << "Failed to initialize GLEW library" << endl;
    glfwTerminate();
    return;
  }
#endif
}

void initScene(void)
{
  graphicsData.world = new cWorld();
  graphicsData.world->m_backgroundColor.setBlack();
  graphicsData.camera = new cCamera(graphicsData.world);
  graphicsData.world->addChild(graphicsData.camera);
  graphicsData.camera->set(cVector3d(3.0, 0.0, 0.0),
                       cVector3d(0.0, 0.0, 0.0),
                       cVector3d(0.0, 0.0, 1.0));
  graphicsData.camera->setClippingPlanes(1.0, 10.0);
  graphicsData.camera->setStereoMode(graphicsData.stereoMode);
  graphicsData.camera->setStereoEyeSeparation(0.03);
  graphicsData.camera->setStereoFocalLength(3.0);
  graphicsData.camera->setMirrorVertical(graphicsData.mirroredDisplay);
  

  graphicsData.light = new cSpotLight(graphicsData.world);
  graphicsData.camera->addChild(graphicsData.light); 
  graphicsData.light->setEnabled(true);
  graphicsData.light->setLocalPos(0.0, 0.5, 0.0);
  graphicsData.light->setDir(-3.0, -0.5, 0.0);
  graphicsData.light->setShadowMapEnabled(true);
  graphicsData.light->m_shadowMap->setQualityLow();
  graphicsData.light->setCutOffAngleDeg(20);
}

void initTask(void)
{
  double workspaceScaleFactor = hapticsData.tool->getWorkspaceScaleFactor();
  double maxStiffness = hapticsData.hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;
  

  graphicsData.object = new cShapeTorus(0.24, 0.50);
  graphicsData.world->addChild(graphicsData.object);
  graphicsData.object->setLocalPos(0.0, 0.0, 0.0);
  graphicsData.object->rotateAboutGlobalAxisDeg(cVector3d(0, 1, 0), 90);
  graphicsData.object->m_material->setStiffness(1.0 * maxStiffness);
  graphicsData.object->m_material->m_ambient.set(0.9f, 0.9f, 0.9f);
  graphicsData.object->m_material->m_diffuse.set(0.9f, 0.9f, 0.9f);
  graphicsData.object->m_material->m_specular.set(1.0f, 1.0f, 1.0f);
  

  cEffectSurface* newEffect = new cEffectSurface(graphicsData.object);
  graphicsData.object->addEffect(newEffect);

}

void resizeWindowCallback(GLFWwindow* a_window, int a_width, int a_height)
{
  graphicsData.width = a_width;
  graphicsData.height = a_height;
}

void errorCallback(int error, const char* errorDescription)
{
  cout << "Error: " << errorDescription << endl;
}

void keySelectCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if ((action != GLFW_PRESS) && (action != GLFW_REPEAT)) {
    return;
  }
  else if ((key == GLFW_KEY_ESCAPE) || (key == GLFW_KEY_Q)) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
  else if(key == GLFW_KEY_F) {
    graphicsData.fullscreen = !graphicsData.fullscreen;
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (graphicsData.fullscreen) {
      glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
      glfwSwapInterval(graphicsData.swapInterval);
    }
    else {
      int w = 0.8 * mode->height;
      int h = 0.5 * mode->height;
      int x = 0.5 * (mode->width - w);
      int y = 0.5 * (mode->height - h);
      graphicsData.width = w;
      graphicsData.height = h;
      graphicsData.xPos = x;
      graphicsData.yPos = y;
      glfwSetWindowMonitor(window, NULL, x, y, w, h, mode->refreshRate);
      glfwSwapInterval(graphicsData.swapInterval);
    }
  }
}

void updateGraphics(void)
{
  graphicsData.world->updateShadowMaps(false, graphicsData.mirroredDisplay);
  graphicsData.camera->renderView(graphicsData.width, graphicsData.height);
  glFinish();
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    cout << "Error: " << gluErrorString(err) << endl;
  }
}
