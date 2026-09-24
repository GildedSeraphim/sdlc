#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define VK_NO_PROTOTYPES
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <libplacebo/log.h>
#include <libplacebo/vulkan.h>
#include <libplacebo/gpu.h>
#include <volk.h>

void err(const char* fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, fmt, ap);
  va_end(ap);
}

void cks(bool result){
  if(!result){
    err("SDL Error: ", SDL_GetError());
    exit(EXIT_FAILURE);
  }
}

void ckp(bool result){
  if(!result){
    err("Libplacebo Error: ");
    exit(EXIT_FAILURE);
  }
}

typedef struct {
  pl_vulkan vk;
  pl_swapchain sw;
} App;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv){
  cks(SDL_SetAppMetadata("sdlc", "0.0.1", "com.gilded.app"));
  cks(SDL_Init(SDL_INIT_VIDEO));

  cks(SDL_Vulkan_LoadLibrary(nullptr));
  auto proc_loader = SDL_Vulkan_GetVkGetInstanceProcAddr();
  cks(proc_loader);
  volkInitializeCustom((PFN_vkGetInstanceProcAddr) proc_loader);
  
  Uint32 inst_ext_count;
  auto inst_exts = SDL_Vulkan_GetInstanceExtensions(&inst_ext_count);

  auto log_params =
    pl_log_params(.log_level = PL_LOG_DEBUG, .log_cb = pl_log_color);
  auto log = pl_log_create(0, log_params);

  auto instance_params = pl_vk_inst_default_params;
  instance_params.get_proc_addr = vkGetInstanceProcAddr;
  instance_params.debug = true;
  instance_params.max_api_version = VK_API_VERSION_1_4;
  instance_params.extensions = inst_exts;
  instance_params.num_extensions = inst_ext_count;
  auto inst = pl_vk_inst_create(log, &instance_params);
  ckp(inst);

  auto window = SDL_CreateWindow("Window", 800, 600, 
        SDL_WINDOW_HIDDEN | SDL_WINDOW_VULKAN | 
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
  cks(window);


  VkSurfaceKHR surface;
  cks(SDL_Vulkan_CreateSurface(window, inst->instance, nullptr, &surface));

  auto vk_params = pl_vulkan_default_params;
  vk_params.get_proc_addr = vkGetInstanceProcAddr;
  vk_params.instance = inst->instance;
  vk_params.surface = surface;
  vk_params.max_api_version = VK_API_VERSION_1_4;
  auto vk = pl_vulkan_create(log, &vk_params);
  ckp(vk);

  auto swapchain = (pl_swapchain)pl_vulkan_create_swapchain(
      vk, pl_vulkan_swapchain_params(.surface = surface));
  ckp(swapchain);
  
  int w, h;
  cks(SDL_GetWindowSizeInPixels(window, &w, &h));

  ckp(pl_swapchain_resize(swapchain, &w, &h));

  App *app = SDL_malloc(sizeof(App));
  cks(app);
  *app = (App){
    .vk = vk,
    .sw = swapchain,
  };

  *appstate = app;

  SDL_ShowWindow(window);
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate){
  App *app = appstate;
  struct pl_swapchain_frame frame;
  if(pl_swapchain_start_frame(app->sw, &frame)) {
    pl_tex_clear(app->vk->gpu, frame.fbo, (float[]){1.0f, 0.0f, 0.5f});
    pl_swapchain_submit_frame(app->sw);
    pl_swapchain_swap_buffers(app->sw);
  };
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
  switch (event->type){
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  default:
    return SDL_APP_CONTINUE;
  }
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){}

