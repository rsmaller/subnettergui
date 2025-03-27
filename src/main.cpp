// ----------------------------------------------------------------------------------------------
//  Table of Contents
// ----------------------------------------------------------------------------------------------
//  1.  Includes and Type Definitions
//  2.  Global Variables
//  3.  Debugging
//  4.  IP Object Wrappers
//  5.  Question Object Wrappers and Conditionals
//  6.  ImGui Callbacks
//  7.  ImGui Wrapper Functions
//  8.  GLFW, OpenGL Initializaiton, and Rendering
//  9.  3D Plotting
//  10. Main Window
//  11. Primary Window Functions
//  12. Info Window Functions
//  13. Entry Point Functions

// ----------------------------------------------------------------------------------------------
// SECTION: Includes and Type Definitions
// ----------------------------------------------------------------------------------------------

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot3d.h"
#include "implot3d_internal.h"
#include "random.h"
#include "subnetterplusplus.hpp"
#include "ipv6tools.hpp"
#include "memsafety.h"
#include <iostream>
#include <string>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <thread>
#include <mutex>
using namespace std;

typedef struct subnettingQuestion {
    IP questionIP;
    SubnetMask questionNetMask;
    string questionString;
    string answer1;
    string answer2;
    string answer3;
    string answer4;
    string answer5;
    string answer6;
    string answer7;
    string answer8;
} subnettingQuestion;

typedef struct classesQuestion {
    IP questionIP;
    SubnetMask classMask;
    string questionString;
    string answer1;
    string answer2;
    string answer3; // not used yet
} classesQuestion;

// ----------------------------------------------------------------------------------------------
// SECTION: Global Variables
// ----------------------------------------------------------------------------------------------

//  window and window control variables to properly exit the program when windows are closed.
GLFWwindow *windowBackend = nullptr;
const int totalNumberOfWindows = 10;
ImGuiContext *currentImGuiContext;
ImGuiID outsideGLFWWindowID;
bool windowsAreOpen[totalNumberOfWindows];
    //  Indices:
    //  0 - Main Window
    //  1 - Export Window
    //  2 - Debug Window
    //  3 - Study Window
    //  4 - IPv6 Window
    //  5 - IP Classes Window
    //  6 - IPv6 Info Window
    //  7 - IP Class Info Window
    //  8 - IP Info Window
    //  9 - Subnet Mask Info Window

//  Debug variables
unsigned long long int frameCount = 0;
unsigned long long int dotCounter = 1;
vector<string> debugEntries;

//  buffers for primary window input data
char *mainInputBuffer1 = (char *)smartCalloc(256, 1); // main IP input
char *mainInputBuffer2 = (char *)smartCalloc(256, 1); // netmask 1 input
char *mainInputBuffer3 = (char *)smartCalloc(256, 1); // netmask 2 input

//  buffers for study input data
char *studyInputBuffer1 = (char *)smartCalloc(256, 1); // main study network IP input
char *studyInputBuffer2 = (char *)smartCalloc(256, 1); // reserved input
char *studyInputBuffer3 = (char *)smartCalloc(256, 1); // reserved input
char *studyInputBuffer4 = (char *)smartCalloc(256, 1); // reserved input
char *studyInputBuffer5 = (char *)smartCalloc(256, 1); // reserved input
char *studyInputBuffer6 = (char *)smartCalloc(256, 1); // reserved input
char *studyInputBuffer7 = (char *)smartCalloc(256, 1); // reserved input
char *studyInputBuffer8 = (char *)smartCalloc(256, 1); // reserved input
subnettingQuestion currentSubnetQuestion = {IP(0), SubnetMask(0), "", "", "", "", "", ""}; // the subnetting question currently displayed on the screen
bool currentSubnetQuestionAnswered = false;

//  buffers for class input data
char *classInputBuffer1 = (char *)smartCalloc(256, 1);
char *classInputBuffer2 = (char *)smartCalloc(256, 1);
char *classInputBuffer3 = (char *)smartCalloc(256, 1);
classesQuestion currentClassQuestion = {IP(0), SubnetMask(0), "", "", "", ""}; // the classes question currently displayed on the screen
bool currentClassQuestionAnswered = false;

//  buffers for export input data
char *exportInputBuffer = (char *)smartCalloc(512, 1); // main input for export path
extern ofstream exportFileStream;
bool exportThreadComplete = false;
mutex exportThreadMutex;
bool currentlyInExportThread = false;

//  IPv6 Tools
char *IPv6InputBuffer = (char *)smartCalloc(256, 1);
char *MACInputBuffer = (char *)smartCalloc(256, 1);
IPv6 currentIPv6Addr;
unsigned short *currentMACAddr;
stringstream randomThreeHextetStream;

//  Primary conditionals
bool subnettingStarted = false;
extern bool subnettingSuccessful;
bool exportButtonPreviouslyPressed = false;
bool graphData = false;
bool nextButtonShown = false;
bool prevButtonShown = false;

// subnet cube variables
ImVec4 plotMenuBackgroundColor(0.1137f, 0.1843f, 0.2863f, 1.0f); // The background of the ImPlot3D menu, not the plot's background!!
float plotBackgroundColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
float axisColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
float bigCubeColor[4] = {0.2f, 0.2f, 0.2f, 1.0f};
float bigCubeEdgeColor[4] = {0.331f, 0.331f, 0.331f, 1.0f};
float smallCubeColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float smallCubeEdgeColor[4] = {0.779f, 0.779f, 0.779f, 1.0f};

// IP cursor variables and arguments
IP IPArg; 
SubnetMask netMaskArg1;
SubnetMask netMaskArg2;
IP currentIP(0); 
extern unsigned long long totalAddedToIP;
int networkMagnitudeDifference = 0;
unsigned long long int totalSubnetsToGenerate = 0;

// ----------------------------------------------------------------------------------------------
// SECTION: Debugging
// ----------------------------------------------------------------------------------------------

void debugLog(string lineToAdd) {
    debugEntries.push_back(lineToAdd);
}

void debug() {
    debugLog("debugging function ran");
    return;
}

void printDebugEntries() {
    for (unsigned long i=0; i<debugEntries.size(); i++) {
        ImGui::Text("%s", (">>> " + debugEntries[i]).c_str());
    }
}

// ----------------------------------------------------------------------------------------------
// SECTION: IP Object Wrappers
// ----------------------------------------------------------------------------------------------

IP constructRandomIP() {
    return IP(getRandomIPNumber());
}

SubnetMask constructRandomSubnetMask() {
    return SubnetMask(getRandomInteger(0, 32));
}

// ----------------------------------------------------------------------------------------------
// SECTION: Question Object Wrappers and Conditionals
// ----------------------------------------------------------------------------------------------

subnettingQuestion randomSubnetQuestion() {
    subnettingQuestion returnValue;
    IP generatedIP = constructRandomIP();
    SubnetMask generatedSubnetMask = constructRandomSubnetMask();
    IP networkIP = generatedIP & generatedSubnetMask;
    returnValue.questionString = "Find all subnet information for a /" + to_string(generatedSubnetMask.networkBits) + " subnet containing the IP address " + generatedIP.IPString + ".";
    returnValue.questionIP = generatedIP;
    returnValue.questionNetMask = generatedSubnetMask;
    returnValue.answer1 = generatedSubnetMask.IPString;
    returnValue.answer2 = to_string(generatedSubnetMask.blockSize);
    returnValue.answer3 = networkIP.IPString;
    returnValue.answer4 = (networkIP + 1).IPString;
    returnValue.answer5 = (networkIP + (int)(generatedSubnetMask.blockSize - 2ULL)).IPString;
    returnValue.answer6 = (networkIP + (int)(generatedSubnetMask.blockSize - 1ULL)).IPString;
    returnValue.answer7 = generatedIP.IPBinaryString;
    returnValue.answer8 = generatedSubnetMask.IPBinaryString;
    return returnValue;
}

classesQuestion randomClassQuestion() {
    classesQuestion returnValue;
    IP generatedIP = constructRandomIP();
    returnValue.questionString = "What class and subnet mask are associated with the IP address " + generatedIP.IPString + " (Enter \"N/A\" if not applicable)?";
    returnValue.answer1 = string(1, generatedIP.IPClass);
    if (generatedIP.IPClass == 'A') {
        returnValue.answer2 = "255.0.0.0";
    } else if (generatedIP.IPClass == 'B') {
        returnValue.answer2 = "255.255.0.0";
    } else if (generatedIP.IPClass == 'C') {
        returnValue.answer2 = "255.255.255.0";
    } else {
        returnValue.answer2 = "N/A";
    }
    return returnValue;
}

void resetCurrentSubnetQuestion() {
    memcpy(studyInputBuffer1, "", 255);
    memcpy(studyInputBuffer2, "", 255);
    memcpy(studyInputBuffer3, "", 255);
    memcpy(studyInputBuffer4, "", 255);
    memcpy(studyInputBuffer5, "", 255);
    memcpy(studyInputBuffer6, "", 255);
    memcpy(studyInputBuffer7, "", 255);
    memcpy(studyInputBuffer8, "", 255);
    currentSubnetQuestionAnswered = false;
}

void showSubnetAnswers() { 
    memcpy(studyInputBuffer1, currentSubnetQuestion.answer1.c_str(), 255);
    memcpy(studyInputBuffer2, currentSubnetQuestion.answer2.c_str(), 255);
    memcpy(studyInputBuffer3, currentSubnetQuestion.answer3.c_str(), 255);
    memcpy(studyInputBuffer4, currentSubnetQuestion.answer4.c_str(), 255);
    memcpy(studyInputBuffer5, currentSubnetQuestion.answer5.c_str(), 255);
    memcpy(studyInputBuffer6, currentSubnetQuestion.answer6.c_str(), 255);
    memcpy(studyInputBuffer7, currentSubnetQuestion.answer7.c_str(), 255);
    memcpy(studyInputBuffer8, currentSubnetQuestion.answer8.c_str(), 255);
    currentSubnetQuestionAnswered = false;
}

void checkSubnetAnswers() {
    currentSubnetQuestionAnswered = true;
}

void resetCurrentClassQuestion() {
    memcpy(classInputBuffer1, "", 255);
    memcpy(classInputBuffer2, "", 255);
    memcpy(classInputBuffer3, "", 255);
    currentClassQuestionAnswered = false;
}

void showClassAnswers() {
    memcpy(classInputBuffer1, currentClassQuestion.answer1.c_str(), 255);
    memcpy(classInputBuffer2, currentClassQuestion.answer2.c_str(), 255);
    // memcpy(classInputBuffer3, currentSubnetQuestion.answer3.c_str(), 255);
    currentClassQuestionAnswered = false;
}

void checkClassAnswers() {
    currentClassQuestionAnswered = true;
}

// ----------------------------------------------------------------------------------------------
// SECTION: ImGui Callbacks
// ----------------------------------------------------------------------------------------------

int argumentChangedCallback(ImGuiInputTextCallbackData *data) {
    (void)data; // data parameter is not used but is required for function datatype to be correct; ignored.
    currentIP.IPAddress.IP32 = 0;
    totalAddedToIP = 256;
    return 1;
}

int subnetQuestionChangedCallback(ImGuiInputTextCallbackData *data) {
    (void)data; // data parameter is not used but is required for function datatype to be correct; ignored.
    currentSubnetQuestionAnswered = false;
    return 1;
}

int classQuestionChangedCallback(ImGuiInputTextCallbackData *data) {
    (void)data;
    currentClassQuestionAnswered = false;
    return 1;
}

int exportChangedCallback(ImGuiInputTextCallbackData *data) {
    (void)data; // data parameter is not used but is required for function datatype to be correct; ignored.
    exportButtonPreviouslyPressed = false;
    return 1;
}

int IPv6ChangedCallback(ImGuiInputTextCallbackData *data) {
    (void)data; // data parameter is not used but is required for function datatype to be correct; ignored.
    currentIPv6Addr = IPv6(IPv6InputBuffer);
    return 1;
}

int MACChangedCallback(ImGuiInputTextCallbackData *data) {
    (void)data; // data parameter is not used but is required for function datatype to be correct; ignored.
    currentMACAddr = IPv6::MACStringToHextets(MACInputBuffer);
    randomThreeHextetStream.str("");
    randomThreeHextetStream << hex << setfill('0') << setw(4) << getRandomShort();
    randomThreeHextetStream << ":";
    randomThreeHextetStream << hex << setfill('0') << setw(4) << getRandomShort();
    randomThreeHextetStream << ":";
    randomThreeHextetStream << hex << setfill('0') << setw(4) << getRandomShort();
    return 1;
}

// ----------------------------------------------------------------------------------------------
// SECTION: ImGui Wrapper Functions
// ----------------------------------------------------------------------------------------------

bool sameLineInIf() {
    ImGui::SameLine();
    return true;
}

// ----------------------------------------------------------------------------------------------
// SECTION: GLFW, OpenGL Initializaiton, and Rendering
// ----------------------------------------------------------------------------------------------

void glfwErrorCallback(int error, const char *msg) {
    if (error != 65544) printf("Error %d: %s\n", error, msg);
}

void windowTerminate() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(currentImGuiContext);
    glfwDestroyWindow(windowBackend);
    glfwTerminate();
    memoryCleanup();
    exit(0);
}

void ImGuiInit() {
    glfwSetErrorCallback(glfwErrorCallback);
    glfwInit();
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    #endif
    windowBackend = glfwCreateWindow(900, 700, "Subnetter++", NULL, NULL);
    glfwMakeContextCurrent(windowBackend);
    float x, y;
    glfwGetMonitorContentScale(monitor, &x, &y);
    glewInit();
    currentImGuiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(currentImGuiContext);
    ImGuiIO &ioRef = ImGui::GetIO();
    ioRef.DisplayFramebufferScale.x = 1.0f;
    ioRef.DisplayFramebufferScale.y = 1.0f;
    ioRef.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ioRef.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(windowBackend, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiViewport* primaryViewPort = ImGui::GetMainViewport();
    primaryViewPort -> PlatformHandle = (void *)windowBackend;
    ImPlot3D::CreateContext();
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        debugLog("OpenGL error in ImGuiInit():" + to_string(error));
        printf("OpenGL Error!: %d\n", error);
    }
}

void startImGuiFrame() {
    glfwMakeContextCurrent(windowBackend);
    glfwPollEvents(); // grabs events every loop. prevents window from not responding
    glClearColor(0.0549019607843f, 0.0549019607843f, 0.0549019607843f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        debugLog("OpenGL error in startImGuiFrame():" + to_string(error));
    }
}

void endImGuiFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    ImGui::EndFrame();
    glfwSwapBuffers(windowBackend); // buffer swapping is necessary to show imgui windows
    glFlush();
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        debugLog("OpenGL error in endImGuiFrame():" + to_string(error));
    }
}

// ----------------------------------------------------------------------------------------------
// SECTION: 3D Plotting
// ----------------------------------------------------------------------------------------------

void resetPlotColors() {
    plotBackgroundColor[0] = 0.0f; 
    plotBackgroundColor[1] = 0.0f; 
    plotBackgroundColor[2] = 0.0f; 
    plotBackgroundColor[3] = 1.0f;
    axisColor[0] = 0.0f; 
    axisColor[1] = 0.0f; 
    axisColor[2] = 0.0f; 
    axisColor[3] = 1.0f;
    bigCubeColor[0] = 0.2f; 
    bigCubeColor[1] = 0.2f; 
    bigCubeColor[2] = 0.2f; 
    bigCubeColor[3] = 1.0f;
    bigCubeEdgeColor[0] = 0.331f; 
    bigCubeEdgeColor[1] = 0.331f; 
    bigCubeEdgeColor[2] = 0.331f; 
    bigCubeEdgeColor[3] = 1.0f;
    smallCubeColor[0] = 1.0f; 
    smallCubeColor[1] = 1.0f; 
    smallCubeColor[2] = 1.0f; 
    smallCubeColor[3] = 1.0f;
    smallCubeEdgeColor[0] = 0.779f; 
    smallCubeEdgeColor[1] = 0.779f; 
    smallCubeEdgeColor[2] = 0.779f; 
    smallCubeEdgeColor[3] = 1.0f;
}

void plotSubnetCubes() {
    float cube1SideLength = (float)cbrt(netMaskArg1.blockSize);
    float vertex1DistanceFromOrigin = (float)cube1SideLength / 2;
    float cube2SideLength = (float)cbrt(netMaskArg2.blockSize);
    float vertex2DistanceFromOrigin = (float)cube2SideLength / 2;
    float vertexDifference = vertex1DistanceFromOrigin - vertex2DistanceFromOrigin;
    ImPlot3DPoint big_cube_vtx[8] = {
        {-vertex1DistanceFromOrigin, -vertex1DistanceFromOrigin, -vertex1DistanceFromOrigin}, // 0: Bottom-back-left
        { vertex1DistanceFromOrigin, -vertex1DistanceFromOrigin, -vertex1DistanceFromOrigin}, // 1: Bottom-back-right
        { vertex1DistanceFromOrigin,  vertex1DistanceFromOrigin, -vertex1DistanceFromOrigin}, // 2: Top-back-right
        {-vertex1DistanceFromOrigin,  vertex1DistanceFromOrigin, -vertex1DistanceFromOrigin}, // 3: Top-back-left
        {-vertex1DistanceFromOrigin, -vertex1DistanceFromOrigin,  vertex1DistanceFromOrigin}, // 4: Bottom-front-left
        { vertex1DistanceFromOrigin, -vertex1DistanceFromOrigin,  vertex1DistanceFromOrigin}, // 5: Bottom-front-right-
        { vertex1DistanceFromOrigin,  vertex1DistanceFromOrigin,  vertex1DistanceFromOrigin}, // 6: Top-front-right
        {-vertex1DistanceFromOrigin,  vertex1DistanceFromOrigin,  vertex1DistanceFromOrigin}, // 7: Top-front-left
    };
    ImPlot3DPoint small_cube_vtx[8] = {
        {-vertex2DistanceFromOrigin - vertexDifference, -vertex2DistanceFromOrigin + vertexDifference, -vertex2DistanceFromOrigin - vertexDifference}, // 0: Bottom-back-left
        { vertex2DistanceFromOrigin - vertexDifference, -vertex2DistanceFromOrigin + vertexDifference, -vertex2DistanceFromOrigin - vertexDifference}, // 1: Bottom-back-right
        { vertex2DistanceFromOrigin - vertexDifference,  vertex2DistanceFromOrigin + vertexDifference, -vertex2DistanceFromOrigin - vertexDifference}, // 2: Top-back-right
        {-vertex2DistanceFromOrigin - vertexDifference,  vertex2DistanceFromOrigin + vertexDifference, -vertex2DistanceFromOrigin - vertexDifference}, // 3: Top-back-left
        {-vertex2DistanceFromOrigin - vertexDifference, -vertex2DistanceFromOrigin + vertexDifference,  vertex2DistanceFromOrigin - vertexDifference}, // 4: Bottom-front-left
        { vertex2DistanceFromOrigin - vertexDifference, -vertex2DistanceFromOrigin + vertexDifference,  vertex2DistanceFromOrigin - vertexDifference}, // 5: Bottom-front-right
        { vertex2DistanceFromOrigin - vertexDifference,  vertex2DistanceFromOrigin + vertexDifference,  vertex2DistanceFromOrigin - vertexDifference}, // 6: Top-front-right
        {-vertex2DistanceFromOrigin - vertexDifference,  vertex2DistanceFromOrigin + vertexDifference,  vertex2DistanceFromOrigin - vertexDifference}, // 7: Top-front-left
    };
    ImGui::PushStyleColor(ImGuiCol_WindowBg, plotMenuBackgroundColor);
    ImGui::SetNextWindowSizeConstraints(ImVec2(100,100), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("3D Subnet Plot", &graphData);
    ImGui::PopStyleColor();
    if (ImPlot3D::BeginPlot(("Big Subnet Cube Side Length: " + to_string(cube1SideLength) + "\nSmall Subnet Cube Side Length: " + to_string(cube2SideLength) + "\n" + to_string(netMaskArg1.blockSize / netMaskArg2.blockSize) + " Small Cubes can fit inside the Big Cube").c_str())) {
        ImPlot3D::SetupAxes("x", "y", "z");
        ImPlot3D::SetupAxesLimits(-vertex1DistanceFromOrigin, vertex1DistanceFromOrigin, -vertex1DistanceFromOrigin, vertex1DistanceFromOrigin, -vertex1DistanceFromOrigin, vertex1DistanceFromOrigin, ImPlot3DCond_Always);
        ImPlot3D::PushStyleColor(ImPlot3DCol_PlotBorder, ImVec4(axisColor[0], axisColor[1], axisColor[2], axisColor[3]));
        ImPlot3D::PushStyleColor(ImPlot3DCol_PlotBg, ImVec4(plotBackgroundColor[0], plotBackgroundColor[1], plotBackgroundColor[2], plotBackgroundColor[3]));
        ImPlot3D::PushStyleColor(ImPlot3DCol_Fill, ImVec4(bigCubeColor[0], bigCubeColor[1], bigCubeColor[2], bigCubeColor[3]));
        ImPlot3D::PushStyleColor(ImPlot3DCol_Line, ImVec4(bigCubeEdgeColor[0], bigCubeEdgeColor[1], bigCubeEdgeColor[2], bigCubeEdgeColor[3]));
        ImPlot3D::PlotMesh(("Big Subnet [/" + to_string(netMaskArg1.networkBits) + "]").c_str(), big_cube_vtx, ImPlot3D::cube_idx, ImPlot3D::CUBE_VTX_COUNT, ImPlot3D::CUBE_IDX_COUNT);
        ImPlot3D::PopStyleColor();
        ImPlot3D::PushStyleColor(ImPlot3DCol_Fill, ImVec4(smallCubeColor[0], smallCubeColor[1], smallCubeColor[2], smallCubeColor[3]));
        ImPlot3D::PushStyleColor(ImPlot3DCol_Line, ImVec4(smallCubeEdgeColor[0], smallCubeEdgeColor[1], smallCubeEdgeColor[2], smallCubeEdgeColor[3]));
        ImPlot3D::PlotMesh(("Small Subnet [/" + to_string(netMaskArg2.networkBits) + "]").c_str(), small_cube_vtx, ImPlot3D::cube_idx, ImPlot3D::CUBE_VTX_COUNT, ImPlot3D::CUBE_IDX_COUNT);
        ImPlot3D::PopStyleColor();
        ImPlot3D::EndPlot();
    }
    ImGui::ColorEdit4("Big Subnet Cube Color", bigCubeColor);
    ImGui::ColorEdit4("Big Subnet Cube Edge Color", bigCubeEdgeColor);
    ImGui::ColorEdit4("Small Subnet Cube Color", smallCubeColor);
    ImGui::ColorEdit4("Small Subnet Cube Edge Color", smallCubeEdgeColor);
    ImGui::ColorEdit4("Axis Color", axisColor);
    ImGui::ColorEdit4("Background Color", plotBackgroundColor);
    if (ImGui::Button("Reset Colors")) resetPlotColors();
    ImGui::End();
}

// ----------------------------------------------------------------------------------------------
// SECTION: Main Window
// ----------------------------------------------------------------------------------------------

void mainWindow() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(100,100), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Subnetter", windowsAreOpen+0, ImGuiWindowFlags_MenuBar);
    ImGui::BeginMenuBar(); // Indented conditionals are part of the menu bar.
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Export")) {debugLog("Export Window Opened"); windowsAreOpen[1] = true;}
            if (ImGui::MenuItem("Open Debug Window")) windowsAreOpen[2] = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Study Subnetting")) windowsAreOpen[3] = true;
            if (ImGui::MenuItem("IP Classes")) windowsAreOpen[5] = true;
            if (ImGui::MenuItem("IPv6 Tools")) windowsAreOpen[4] = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("IP Info")) windowsAreOpen[8] = true;
            if (ImGui::MenuItem("IP Class Info")) windowsAreOpen[7] = true;
            if (ImGui::MenuItem("IPv6 Info")) windowsAreOpen[6] = true;
            if (ImGui::MenuItem("Subnet Mask Info")) windowsAreOpen[9] = true;
            ImGui::EndMenu();
        }
    ImGui::EndMenuBar();
    ImGui::InputText("IP", mainInputBuffer1, 255, ImGuiInputTextFlags_CallbackEdit, argumentChangedCallback);
    ImGui::InputText("Netmask 1", mainInputBuffer2, 255, ImGuiInputTextFlags_CallbackEdit, argumentChangedCallback);
    ImGui::InputText("Netmask 2", mainInputBuffer3, 255, ImGuiInputTextFlags_CallbackEdit, argumentChangedCallback);
    ImGui::Checkbox("Binary", &binaryFlag);
    ImGui::SameLine();
    ImGui::Checkbox("Reverse", &reverseFlag);
    ImGui::SameLine();
    ImGui::Checkbox("Extra Info", &debugFlag);
    ImGui::SameLine();
    ImGui::Checkbox("3D Plotting", &graphData);
    if (ImGui::Button("Begin Subnetting")) {
        if (subnettingStarted == false) debugLog("Subnetting Started");
        subnettingStarted = true;
    } ImGui::SameLine();
    if (ImGui::Button("Stop Subnetting")) {
        if (subnettingStarted == true) debugLog("Subnetting Stopped");
        subnettingStarted = false;
    } ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        debugLog("Refresh Button Pressed");
        currentIP.IPAddress.IP32 = 0;
        totalAddedToIP = 256;
    }
    ImGui::SameLine();
    if (ImGui::Button("Generate Random Subnet")) {
        currentIP.IPAddress.IP32 = 0;
        totalAddedToIP = 256;
        memcpy(mainInputBuffer1, constructRandomIP().IPString.c_str(), 255);
        memcpy(mainInputBuffer2, to_string(getRandomCIDR()).c_str(), 255);
        memcpy(mainInputBuffer3, to_string(getRandomCIDR()).c_str(), 255);
        subnettingStarted = false;
    }
    if (windowsAreOpen[2] && sameLineInIf() && ImGui::Button("Clear Debug Log")) {
        debugEntries.clear();
    }
    if (windowsAreOpen[2]) {
        ImGui::SameLine();
        ImGui::Text("%s", ("Number of Debug Entries: " + to_string(debugEntries.size())).c_str());
    }
    if (ImGui::Button("Go to Start")) {
        currentIP.IPAddress.IP32 = 0;
        totalAddedToIP = 256;
    }
    ImGui::SameLine();
    if (ImGui::Button("Previous 256 Subnets") && (totalAddedToIP > 256)) {
        if (subnettingStarted) debugLog("Previous Button Pressed");
        if (totalAddedToIP > 256) {currentIP -= 256 * (unsigned int)netMaskArg2.blockSize;
        totalAddedToIP -= 256;}
    }
    ImGui::SameLine();
    if (ImGui::Button("Next 256 Subnets") && (totalAddedToIP < totalSubnetsToGenerate)) {
        if (subnettingStarted) debugLog("Next Button Pressed");
        currentIP += 256 * (unsigned int)netMaskArg2.blockSize;
        totalAddedToIP += 256;
    }
    ImGui::SameLine();
    if (ImGui::Button("Go to End") && totalAddedToIP != totalSubnetsToGenerate && totalSubnetsToGenerate > 256) {
        currentIP += (unsigned int)(totalSubnetsToGenerate- 256) * (unsigned int)netMaskArg2.blockSize;
        totalAddedToIP = totalSubnetsToGenerate - (totalSubnetsToGenerate % 256);
    }
    if (!subnettingStarted) {
        ImGui::End();
        return;
    }
    ImGui::BeginChild("ScrollWheel");
    SubnetMask swapMask;
    IPArg = IP(mainInputBuffer1);
    if (strcmp(mainInputBuffer2, "") && !strcmp(mainInputBuffer3, "")) {
        netMaskArg1 = SubnetMask(mainInputBuffer2);
        netMaskArg2 = SubnetMask(mainInputBuffer2);
    } else if (!strcmp(mainInputBuffer2, "") && strcmp(mainInputBuffer3, "")) {
        netMaskArg1 = SubnetMask(mainInputBuffer3);
        netMaskArg2 = SubnetMask(mainInputBuffer3);
    } else {
        netMaskArg1 = SubnetMask(mainInputBuffer2);
        netMaskArg2 = SubnetMask(mainInputBuffer3);
    }
    if (netMaskArg1.networkBits > netMaskArg2.networkBits) { // If netMaskArg1's CIDR mask is greater than netMaskArg2's, swap them to ensure that netMaskArg1's CIDR mask is >= to netMaskArg2's.
        swapMask = netMaskArg1;
        netMaskArg1 = netMaskArg2;
        netMaskArg2 = swapMask;
    }
    networkMagnitudeDifference = netMaskArg1.hostBits - netMaskArg2.hostBits;
    totalSubnetsToGenerate = 1ULL<<networkMagnitudeDifference;
    if (debugFlag) {
        ImGui::Text("%s", ("Added total in main: " + to_string(totalAddedToIP)).c_str()); 
        ImGui::Text("%s", ("Subnets to generate: " + to_string(totalSubnetsToGenerate)).c_str()); 
        ImGui::Text("%s", ("Magnitude difference: " + to_string(networkMagnitudeDifference)).c_str());
        ImGui::Text("%s", ("Current IP: " + currentIP.IPString).c_str());
    }
    if (currentIP.IPAddress.IP32 == 0) {
        currentIP = IPArg & netMaskArg1;
    }
    timedVLSM(currentIP, netMaskArg1, netMaskArg2, false);
    ImGui::EndChild();
    ImGui::End();
}

// ----------------------------------------------------------------------------------------------
// SECTION: Primary Window Functions
// ----------------------------------------------------------------------------------------------

void studyWindow() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(100,100), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Study", windowsAreOpen+3);
    if (currentSubnetQuestion.questionString.compare("")) { 
        ImGui::Text("%s", currentSubnetQuestion.questionString.c_str());
        ImGui::InputText("Subnet Mask", studyInputBuffer1, 255, ImGuiInputTextFlags_CallbackEdit, subnetQuestionChangedCallback);
        ImGui::InputText("Block Size", studyInputBuffer2, 255, ImGuiInputTextFlags_CallbackEdit, subnetQuestionChangedCallback);
        ImGui::InputText("Network Address", studyInputBuffer3, 255, ImGuiInputTextFlags_CallbackEdit, subnetQuestionChangedCallback);
        ImGui::InputText("First Usable Address in Subnet", studyInputBuffer4, 255, ImGuiInputTextFlags_CallbackEdit, subnetQuestionChangedCallback);
        ImGui::InputText("Last Usable Address in Subnet", studyInputBuffer5, 255, ImGuiInputTextFlags_CallbackEdit, subnetQuestionChangedCallback);
        ImGui::InputText("Broadcast Address", studyInputBuffer6, 255, ImGuiInputTextFlags_CallbackEdit, subnetQuestionChangedCallback);
        ImGui::InputText(("Binary for " + currentSubnetQuestion.questionIP.IPString).c_str(), studyInputBuffer7, 255, ImGuiInputTextFlags_CallbackEdit, subnetQuestionChangedCallback);
        ImGui::InputText(("Binary for /" + to_string(currentSubnetQuestion.questionNetMask.networkBits) + " Mask").c_str(), studyInputBuffer8, 255, ImGuiInputTextFlags_CallbackEdit, subnetQuestionChangedCallback);
        if (ImGui::Button("Check Answers")) {checkSubnetAnswers();}
        ImGui::SameLine();
        if (ImGui::Button("Show Answers")) {showSubnetAnswers();}
        ImGui::SameLine();
        if (ImGui::Button("Hide Answers")) {resetCurrentSubnetQuestion();}
        ImGui::SameLine();
    }
    if (ImGui::Button("Generate New Question")) {
        currentSubnetQuestion = randomSubnetQuestion();
        resetCurrentSubnetQuestion();
    }
    if (currentSubnetQuestionAnswered) {
        if (!currentSubnetQuestion.answer1.compare(studyInputBuffer1)) {ImGui::Text("Subnet Mask is correct!");} else {ImGui::Text("Subnet Mask is incorrect.");}
        if (!currentSubnetQuestion.answer2.compare(studyInputBuffer2)) {ImGui::Text("Block Size is correct!");} else {ImGui::Text("Block Size is incorrect.");}
        if (!currentSubnetQuestion.answer3.compare(studyInputBuffer3)) {ImGui::Text("Network Address is correct!");} else {ImGui::Text("Network Address is incorrect.");}
        if (!currentSubnetQuestion.answer4.compare(studyInputBuffer4)) {ImGui::Text("First Usable Address is correct!");} else {ImGui::Text("First Usable Address is incorrect.");}
        if (!currentSubnetQuestion.answer5.compare(studyInputBuffer5)) {ImGui::Text("Last Usable Address is correct!");} else {ImGui::Text("Last Usable Address is incorrect.");}
        if (!currentSubnetQuestion.answer6.compare(studyInputBuffer6)) {ImGui::Text("Broadcast Address is correct!");} else {ImGui::Text("Broadcast Address is incorrect.");}
        if (!currentSubnetQuestion.answer7.compare(studyInputBuffer7)) {ImGui::Text("Binary for Provided IP is correct!");} else {ImGui::Text("Binary for Provided IP is incorrect.");}
        if (!currentSubnetQuestion.answer8.compare(studyInputBuffer8)) {ImGui::Text("Binary for Subnet Mask is correct!");} else {ImGui::Text("Binary for Subnet Mask is incorrect.");}
    }
    ImGui::End();
    return;
}

void IPClassWindow() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(100,100), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("IP Classes", windowsAreOpen+5);
    if (currentClassQuestion.questionString.compare("")) {
        ImGui::Text("%s", currentClassQuestion.questionString.c_str());
        ImGui::InputText("IP Class", classInputBuffer1, 255, ImGuiInputTextFlags_CallbackEdit, classQuestionChangedCallback);
        ImGui::InputText("Subnet Mask", classInputBuffer2, 255, ImGuiInputTextFlags_CallbackEdit, classQuestionChangedCallback);
    }
    if (ImGui::Button("Check Answers")) {checkClassAnswers();}
    ImGui::SameLine();
    if (ImGui::Button("Show Answers")) {showClassAnswers();}
    ImGui::SameLine();
    if (ImGui::Button("Hide Answers")) {resetCurrentClassQuestion();}
    ImGui::SameLine();
    if (ImGui::Button("Generate New Question")) {
        currentClassQuestion = randomClassQuestion();
        resetCurrentClassQuestion();
    }
    if (currentClassQuestionAnswered) {
        if (!currentClassQuestion.answer1.compare(classInputBuffer1)) {ImGui::Text("IP Class is correct!");} else {ImGui::Text("IP Class is incorrect.");}
        if (!currentClassQuestion.answer2.compare(classInputBuffer2)) {ImGui::Text("Subnet Mask is correct!");} else {ImGui::Text("Subnet Mask is incorrect.");} 
    }
    ImGui::End();
    return;
}

void IPv6Window() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(100,100), ImVec2(FLT_MAX, FLT_MAX));
    string EUIString = IPv6::MACStringToEUIString(MACInputBuffer);
    string fullIPv6EUIString = IPv6::IPv6Sanitize("fe80:" + randomThreeHextetStream.str() + ":" + EUIString);
    currentIPv6Addr = IPv6(IPv6InputBuffer);
    ImGui::Begin("IPv6 Tools", windowsAreOpen+4);
    ImGui::InputText("IPv6 Address", IPv6InputBuffer, 255, ImGuiInputTextFlags_CallbackEdit, IPv6ChangedCallback);
    if (ImGui::Button("Generate Random IP Address")) {
        unsigned short *randomIPV6Num = getRandomIPv6Number();
        currentIPv6Addr = IPv6(randomIPV6Num);
        memcpy(IPv6InputBuffer, currentIPv6Addr.IPv6String.c_str(), 255);
        free(randomIPV6Num);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear IPv6")) {
        memcpy(IPv6InputBuffer, "", 255);
    }
    ImGui::Text("%s", ("Shorthand:    " + currentIPv6Addr.shortenedIPv6String).c_str());
    ImGui::Text("%s", ("Full:         " + currentIPv6Addr.IPv6String).c_str());
    ImGui::Text("%s", ("Prefix:       " + currentIPv6Addr.IPv6String.substr(0, 14)).c_str());
    ImGui::Text("%s", ("Subnet ID:                   " + currentIPv6Addr.IPv6String.substr(15, 4)).c_str());
    ImGui::Text("%s", ("Interface ID:                     " + currentIPv6Addr.IPv6String.substr(20, 19)).c_str());
    ImGui::Text("%s", ("Address Type: " + currentIPv6Addr.type()).c_str());
    ImGui::Text(" ");
    ImGui::Text("Link-local IPv6 interface IDs can be generated using a device's MAC address via a process called EUI64.");
    ImGui::InputText("MAC Address", MACInputBuffer, 255, ImGuiInputTextFlags_CallbackEdit, MACChangedCallback);
    if (ImGui::Button("Generate Random MAC Address")) {
        memcpy(MACInputBuffer, IPv6::MACHextetsToString(getRandomMACNumber()).c_str(), 255);
        MACChangedCallback(NULL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear MAC")) {
        memcpy(MACInputBuffer, "", 255);
        MACChangedCallback(NULL);
        randomThreeHextetStream.str("0000:0000:0000");
    }
    ImGui::Text("%s", ("EUI64 Interface ID:                          " + EUIString).c_str());
    ImGui::Text("%s", ("Full IPv6 EUI64 Example: " + fullIPv6EUIString).c_str());
    ImGui::End();
}

void exportWindow() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(100,100), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Export...", windowsAreOpen+1);
    ImGui::InputText("Export Path", exportInputBuffer, 511, ImGuiInputTextFlags_CallbackEdit, exportChangedCallback); // if this input is changed, reset the export success message.
    if (ImGui::Button("Export")) {
        if (currentlyInExportThread) { // The export button does nothing if there is an ongoing export process.
            ImGui::End();
            return;
        }
        unique_lock exportThreadLock(exportThreadMutex); // Lock the exportThreadMutex so exportThreadComplete is not accessed by an ongoing thread while changing its value to false.
        exportThreadComplete = false;
        exportThreadLock.unlock();
        exportButtonPreviouslyPressed = true;
        IPArg = IP(mainInputBuffer1);
        netMaskArg1 = SubnetMask(mainInputBuffer2);
        netMaskArg2 = SubnetMask(mainInputBuffer3);
        string exportString = string(exportInputBuffer);
        debugLog("Export Button pressed with path " + exportString);
        string mainPath = exportString.substr(0, exportString.find_last_of("\\/") + 1);
        string resultFile = exportString.substr(exportString.find_last_of("\\/") + 1, exportString.length() - 1);
        if (!(filesystem::exists(filesystem::path(mainPath)) && resultFile.contains(".txt"))) { // Do not continue through the rest of the export process if the path is invalid.
            exportThreadLock.lock();
            exportThreadComplete = false;
            exportThreadLock.unlock();
            debugLog("Main Path: " + mainPath);
            debugLog("Result File: " + resultFile);
            ImGui::Text("File could not be exported. Is the path correct?");
            ImGui::End();
            return;
        }
        debugLog("Entered path exists");
        debugLog("Main Path: " + mainPath);
        debugLog("Result File: " + resultFile);
        exportFileStream.open(exportString);
        debugLog("File created!");
        thread exportThread = thread(
            []() -> void { // Lambda function that runs in a detached thread
                currentlyInExportThread = true;
                timedVLSM(IPArg, netMaskArg1, netMaskArg2, true);
                exportFileStream.close();
                unique_lock exportThreadLock(exportThreadMutex); 
                exportThreadComplete = true; 
                currentlyInExportThread = false;
                exportThreadLock.unlock();
            }
        );
        exportThread.detach();
        debugLog("File exported!");
    }
    if (!exportButtonPreviouslyPressed && !currentlyInExportThread) { // Don't show the "Exporting..." message if the export button has not been pressed and there is no export thread.
        ImGui::End();
        return;
    }
    unique_lock exportThreadLock(exportThreadMutex);
    if (exportThreadComplete) {
        ImGui::Text("File exported!");
    } else {
        ImGui::Text("%s", ("Exporting" + string(dotCounter, '.')).c_str());
    }
    ImGui::End();
    return;
}

void debugWindow() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(100,100), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Debug Log", windowsAreOpen+2);
    ImGui::Text("%s", ("Frame Count: " + to_string(frameCount)).c_str());
    ImGui::Text("%s", ("Dot Count: " + to_string(dotCounter)).c_str());
    ImGui::BeginChild("ScrollWheel");
    printDebugEntries();
    ImGui::EndChild();
    ImGui::End();
}

// ----------------------------------------------------------------------------------------------
// SECTION: Info Window Functions
// ----------------------------------------------------------------------------------------------

void IPv6InfoWindow() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(100,100), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("IPv6 Info", windowsAreOpen+6);
    ImGui::BeginChild("ScrollWheel");
    ImGui::Text("IPv6 addresses are 128-bit addresses which are split into groups called hextets; hextets are 16 bits each.");
    ImGui::Text("Therefore, an IPv6 address has 8 hextets in it. Each hextet is represented as a 4 digit hexadecimal number.");
    ImGui::Text("Furthermore, each hextet is separated by a colon, meaning there are 7 colons in a full-length IPv6 address.");
    ImGui::Text("   For example: a0f0:0000:0000:43a2:02d8:2b0f:764c:f47a");
    ImGui::Text(" ");
    ImGui::Text("IPv6 sections are split up into three sections: the prefix, the subnet ID, and the interface ID.");
    ImGui::Text("The prefix is the first three hextets of an IPv6 address. The prefix for the above address is a0f0:0000:0000.");
    ImGui::Text("The subnet ID is the fourth hextet of the address. The subnet ID for the above address is 43a2.");
    ImGui::Text("The interface ID is the last four hextets of the address, or 02d8:2b0f:764c:f47a.");
    ImGui::Text("The prefix determines the type of the IPv6 address (see below).");
    ImGui::Text("The subnet ID determines network to which the IPv6 address belongs.");
    ImGui::Text("The interface ID is the identifier portion which is unique to a device on the subnet.");
    ImGui::Text(" ");
    ImGui::Text("However, IPv6 addresses can be shortened. Each hextet can ignore any leading zeroes.");
    ImGui::Text("   Any number can be written without leading zeroes in any number system. 0100 is always the same as 100.");
    ImGui::Text("   For example: a0f0:0:0:43a2:2d8:2b0f:764c:f47a");
    ImGui::Text("The longest chain of hextets which only contain zeroes can also be replaced with a double colon.");
    ImGui::Text("   For example: a0f0::43a2:2d8:2b0f:764c:f47a");
    ImGui::Text(" ");
    ImGui::Text("There are also various types of IPv6 addresses. The types of IPv6 addresses each fall into an address range.");
    ImGui::Text("Each address type is either unicast, multicast, or anycast.");
    ImGui::Text("   Unicast addresses are used to communicate with a single device.");
    ImGui::Text("   Multicast addresses are used to communicate with many devices.");
    ImGui::Text("   Anycast addresses are used to communicate with the closest reachable device in a multicast group.");
    ImGui::Text(" ");
    ImGui::Text("Each address range is in shortened form and with the address block written in CIDR notation.");
    ImGui::Text("More information about CIDR can be found in the IP classes info block.");
    ImGui::Text("The address ranges are as follows: ");
    ImGui::Text("   Assigned multicast - FF00:: (/8)");
    ImGui::Text("   Solicited-node multicast - FF02::1:FF00:0 (/104)");
    ImGui::Text("   Loopback Unicast - :: (/127)");
    ImGui::Text("   Link-local Unicast/Anycast - FE80:: (/10)");
    ImGui::Text("   Site-local Unicast/Anycast - FEC0:: (/10)");
    ImGui::Text("   Aggregatable Global Unicast - 2001:: (/16) or 2002:: (/16) or 3FFE:: (/16)");
    ImGui::Text(" ");
    ImGui::Text("Also, IPv4 addresses can be written in an IPv6 format. Because IPv4 addresses are 32 bits,");
    ImGui::Text("They can be stored in the last two hextets of an IPv6 address.");
    ImGui::Text("To make it clear that an IPv4 address is being stored there, the preceding six hextets must be zero.");
    ImGui::Text("Therefore, the address range for IPv4-compatible IPv6 addresses is :: (/96).");
    ImGui::Text(" ");
    ImGui::EndChild();
    ImGui::End();
}

void IPClassInfoWindow() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(100,100), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("IP Class Info", windowsAreOpen+7);
    ImGui::BeginChild("ScrollWheel");
    ImGui::Text("In the older days of IPv4 addressing, a method of organization called classful addressing was used.");
    ImGui::Text("IP classes were a way of delegating subnets groups of IPv4 addresses into categories with designated subnet masks.");
    ImGui::Text("There were three main classes used in everyday addressing: A, B, and C.");
    ImGui::Text("   Class A addresses were assigned a subnet mask of 255.0.0.0 or /8 in CIDR notation.");
    ImGui::Text("   Class B addresses were assigned a subnet mask of 255.255.0.0 or /16 in CIDR notation.");
    ImGui::Text("   Class C addresses were assigned a subnet mask of 255.255.255.0 or /24 in CIDR notation.");
    ImGui::Text("   There are also class D addresses, used for multicast, and experimental class E addresses.");
    ImGui::Text(" ");
    ImGui::Text("Because IP classes are organized groups of IP addresses, they can be written as subnets!!");
    ImGui::Text("The groups of IP addresses for each IP class is as follows:");
    ImGui::Text("   Class A addresses range from 0.0.0.0 to 127.255.255.255. This would be 0.0.0.0/1 as a subnet.");
    ImGui::Text("   Class B addresses range from 128.0.0.0 to 191.255.255.255. This would be 128.0.0.0/2 as a subnet.");
    ImGui::Text("   Class C addresses range from 192.0.0.0 to 223.255.255.255. This owuld be 192.0.0.0/3 as a subnet.");
    ImGui::Text("   Class D addresses range from 224.0.0.0 to 239.255.255.255. This would be 224.0.0.0/4 as a subnet.");
    ImGui::Text("   Class E addresses range from 240.0.0.0 to 255.255.255.255. This would be 240.0.0.0/4 as a subnet.");
    ImGui::Text(" ");
    ImGui::Text("While it is important to understand IP classes and how they work, they are not used much anymore.");
    ImGui::Text("IP classes have been replaced by CIDR, or Classless Inter-Domain Routing.");
    ImGui::Text("CIDR is a subnet notation for IP addresses where any IP address can be part of a network with any subnet mask.");
    ImGui::Text("CIDR introduced a new notation for subnet masks as well. Instead of writing them like IP addresses,");
    ImGui::Text("They can be written as a slash followed by the count of the number of network bits in the subnet mask.");
    ImGui::Text("   For example, the subnet mask 255.255.255.0 can be written in binary as 11111111.11111111.11111111.00000000.");
    ImGui::Text("   There are 24 ones in that mask that represent the network portion of the subnet mask.");
    ImGui::Text("   In CIDR notation, 255.255.255.0 would therefore be written as /24.");
    ImGui::Text("The IP class address groups and the IPv6 type groups both use this notation to represent groups of IP addresses.");
    ImGui::Text(" ");
    ImGui::EndChild();
    ImGui::End();
}

void IPInfoWindow() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(100,100), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("IP Info", windowsAreOpen+8);
    ImGui::BeginChild("ScrollWheel");
    ImGui::Text("IP addresses are very similar to an address you might find for a home. 30 1st Street in New York City shows you");
    ImGui::Text("the general area of the destination's location, New York City, as well as the local area, 1st Street,");
    ImGui::Text("and its exact location on that street, 30.");
    ImGui::Text("IP addresses function the same way. The IPv4 address 112.14.8.2 might not seem meaningful, but it is a roadmap to a");
    ImGui::Text("device on some network. IP addresses contain a network portion, which is analogous to a city or a street, and a");
    ImGui::Text("host portion, which is analogous to the exact location of a house on a street. Both pieces of information are");
    ImGui::Text("necessary to locate a device.");
    ImGui::Text("More information on network and host portions can be found in the subnet mask info block. This info block, however, will discuss");
    ImGui::Text("the nature and format of IP addresses.");
    ImGui::Text(" ");
    ImGui::Text("An IPv4 address is just a number. It is a number that is usually written as four numbers separated by dots, but in reality, it is");
    ImGui::Text("simply one number between 0 and 4,294,967,295.");
    ImGui::Text("The reason for this oddly specific number is that IP addresses are located on computers and must be used in ways a computer can understand.");
    ImGui::Text(" ");
    ImGui::Text("Computers do not use the number system we humans are familiar with; computers use a binary system, or base 2 system, instead of the");
    ImGui::Text("decimal system most of the world uses, otherwise known as the base 10 system.");
    ImGui::Text("The word 'base' has a few different characteristics that make it significant.");
    ImGui::Text("   Firstly, the base refers to how many different numbers can be put in a single digit in any number system.");
    ImGui::Text("   In base 10, the numbers are 0 through 9. 0 through 9 makes up 10 numbers in total.");
    ImGui::Text("   Secondly, the base also determines the values of 'places', or digits, of numbers in a number system.");
    ImGui::Text("   In base 10, we have the 1's place, the 10's place, the 100's place, the 1,000's place, and so on.");
    ImGui::Text("Conversely:");
    ImGui::Text("   In base 2, the numbers 0 through 1 are used, making up 2 numbers in total.");
    ImGui::Text("   Furthermore, the 'places' binary uses are exponents of 2 instead of exponents of 10.");
    ImGui::Text("   Binary has the 1's place, 2's place, 4's place, 8's place, 16's place, and so on.");
    ImGui::Text("   The value of each digit essentially doubles.");
    ImGui::Text(" ");
    ImGui::Text("Fortunately, everything else about the binary number system is the exact same as the decimal number system. They follow all the same conventions,");
    ImGui::Text("and only the base is different.");
    ImGui::Text("Below are examples of how numbers would be read in both the regular decimal system and binary system.");
    ImGui::Text("Keep in mind for both decimal and binary, the 1's place is the rightmost digit.");
    ImGui::Text("The decimal number 142:");
    ImGui::Text("   The 1's place is 2. That means the value at the 1's place is the place * 2, or 1 * 2 => 2.");
    ImGui::Text("   The 10's place is 4. That means the value at the 10's place is the place * 4, or 10 * 4 => 40.");
    ImGui::Text("   The 100's place is 1. That means the value as the 100's place is the place * 1, or 100 * 1 => 100.");
    ImGui::Text("   These values are then added together to get 100 + 40 + 2, or 142.");
    ImGui::Text("   In everyday life, we usually skip this calculation because we would speak it.");
    ImGui::Text("   \"One hundred forty-two\" is the same as 100 + 40 + 2.");
    ImGui::Text("In binary, we cannot write the same number as \"142\". It would instead have to be written as \"10001110\":");
    ImGui::Text("   The 1's place is 0. That means the value at the 1's place is the place * 0, or 1 * 0 => 0.");
    ImGui::Text("   The 2's place is 1. The calculation for that digit would be 2 * 1 => 2.");
    ImGui::Text("   The 4's place is 1. The calculation would be 4 * 1 => 4.");
    ImGui::Text("   The 8's place is also 1. The calculation would be 8 * 1 => 8.");
    ImGui::Text("   The 16's place is 0. The calculation would be 16 * 0 => 0.");
    ImGui::Text("   The 32's place is 0. The calculation would be 32 * 0 => 0.");
    ImGui::Text("   The 64's place is 0. The calculation would be 64 * 0 => 0.");
    ImGui::Text("   The 128's place is 1. The calculation would be 128 * 1 => 128.");
    ImGui::Text("   Just like in decimal, these values are added together to get 128 + 8 + 4 + 2, which is 142.");
    ImGui::Text("   Based on this calculation, it could be said that \"10001110\" is the binary representation of \"142\".");
    ImGui::Text(" ");
    ImGui::Text("IPv4 addresses are binary numbers. They are stored in 32 \"bits\", meaning 32 binary digits.");
    ImGui::Text("Therefore, an IPv4 address can be anything from \"00000000000000000000000000000000\" to \"11111111111111111111111111111111\".");
    ImGui::Text("The above numbers converted to decimal are 0 to 4,294,967,295.");
    ImGui::Text(" ");
    ImGui::Text("But how can a 32-bit binary number be written as something like 112.14.8.2?");
    ImGui::Text("Simply put, each 32-bit address is separated into four 8-bit sections, each section separated by a dot.");
    ImGui::Text("Each section is then converted into our decimal number system.");
    ImGui::Text("The IP address 112.114.8.2 would originally be stored in a computer as \"01110000000011100000100000000010\".");
    ImGui::Text("Being separated into four 8-bit sections, it would become: \"01110000.00001110.00001000.00000010\".");
    ImGui::Text("Finally, converting each 8-bit section into decimal would result in the IP address shown to us as 112.14.8.2.");
    ImGui::Text(" ");
    ImGui::EndChild();
    ImGui::End();
}

void subnetMaskInfoWindow() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(100,100), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Subnet Mask Info", windowsAreOpen+9);
    ImGui::BeginChild("ScrollWheel");
    ImGui::Text("Subnet masks are 32-bit binary numbers which use almost the same format as IPv4 addresses.");
    ImGui::Text("For example, a subnet mask might be written as 255.255.255.0, which is also a valid IP address.");
    ImGui::Text("The meaning of 255.255.255.0 as a subnet mask versus an IP address is very different, however.");
    ImGui::Text("As mentioned in the IP info block, IP addresses have a network and host portion. The network portion is analogous to the street");
    ImGui::Text("a device resides on, while the host portion is more like the house number for a device on that street of devices.");
    ImGui::Text("For the IPv4 address 192.168.0.1, how would we know which part of the address is the \"street\" versus the \"house number\"?");
    ImGui::Text("In short, we can't. An IPv4 address by itself does not have a specific network or host portion. It is the subnet mask's job to");
    ImGui::Text("specify that information.");
    ImGui::Text(" ");
    ImGui::Text("Subnet masks specify that information in an interesting way. A subnet mask is always a chain of ones followed by a chain of zeroes in binary.");
    ImGui::Text("For example, with the dots separating each set of 8 bits like in IPv4, a subnet mask might be \"11111111.11111111.11111111.00000000\".");
    ImGui::Text("That binary translates to the subnet mask 255.255.255.0. However, \"01010101.11000000.00110001.00000000\" is not a valid subnet mask.");
    ImGui::Text("The reason that subnet masks are formatted this way is because the chain of ones represents the network portion and the chain of zeroes");
    ImGui::Text("represents the host portion. In other words, the chain of ones and zeroes say how many binary digits should be used to represent the");
    ImGui::Text("street versus the house number for an IPv4 address.");
    ImGui::Text(" ");
    ImGui::Text("But what does this all mean? While it's nice to have more data given to the street name or the house number, what is the impact of changing");
    ImGui::Text("those things? Well, it is important to keep in mind that the \"street\" we are working with is actually a network; a network is simply a group of");
    ImGui::Text("devices that can communicate with each other. If you give less space to the network portion, the number of street names, which are technically network IDs,");
    ImGui::Text("that can be generated is smaller. However, shortening the network portion means the host portion will get the leftover space and become larger, meaning");
    ImGui::Text("the number of houses, or devices, on the network will be larger. In other words, making fewer networks means each individual network will be larger.");
    ImGui::Text(" ");
    ImGui::Text("Here is a practical example:");
    ImGui::Text("   For the sake of example, let's say we have an IP address 192.168.1.1 with a subnet mask of 255.255.255.0.");
    ImGui::Text("   The subnet mask can also be written as \"11111111.11111111.11111111.00000000\"");
    ImGui::Text("   It is clear that there are 24 ones and 8 zeroes present in this subnet mask, meaning the network portion of this");
    ImGui::Text("   subnet mask is 24 bits long and the host portion is 8 bits long.");
    ImGui::Text("   While this data is nice to know by itself, it does not tell us anything about the network that the IP address 192.168.1.1 resides on.");
    ImGui::Text("   How many networks can be generated with 24 network bits, and more importantly, how many hosts can exist on each network with 8 host bits?");
    ImGui::Text(" ");
    ImGui::Text("To answer the first question this example presents, it is important to understand the question \"How many different numbers can be generated with X amount of digits?\"");
    ImGui::Text("In the decimal number system, we use base 10, meaning that each digit in our number system has 10 different possibilities: 0 through 9.");
    ImGui::Text("Therefore, in a three digit number, we can have 000-999. 0 through 999 is 1,000 numbers. If we add a digit, the number of possibilties multiplies by 10: 10,000.");
    ImGui::Text("0 through 9,999 is 10,000 numbers.");
    ImGui::Text("And so, a pattern emerges. Each digit we add lets us create 10 times as many numbers.");
    ImGui::Text("The solution to this question, therefore, is that we multiply by 10 for each digit, or 10 time 10 X times. This is exponentiation, so we can rewrite the problem as follows:");
    ImGui::Text("The number of possibile numbers that can be generated with X amount of digits in the decimal number system is 10 to the X.");
    ImGui::Text(" ");
    ImGui::Text("This may not yet seem related to our original problem of figuring out what network and host portions mean. We must rephrase the problem in binary.");
    ImGui::Text("In the binary number system, we use base 2, meaning each digit in binary has 2 different possibilities: 0 and 1.");
    ImGui::Text("In a three digit number, we can have the binary numbers 000-111.");
    ImGui::Text("However, let us take care to avoid confusion. 000 through 111 is NOT like below:");
    ImGui::Text("   000 -> 0");
    ImGui::Text("   001 -> 1");
    ImGui::Text("   002 -> 2");
    ImGui::Text("   003 -> 3");
    ImGui::Text("   ...");
    ImGui::Text("This list is using a decimal system, which is different from binary.");
    ImGui::Text("Binary is more like this:");
    ImGui::Text("   000 -> 0");
    ImGui::Text("   001 -> 1");
    ImGui::Text("   010 -> 2");
    ImGui::Text("   011 -> 3");
    ImGui::Text("   100 -> 4");
    ImGui::Text("   101 -> 5");
    ImGui::Text("   110 -> 6");
    ImGui::Text("   111 -> 7");
    ImGui::Text(" ");
    ImGui::Text("It may be observed now that there are 8 different numbers that can be made with 3 binary digits, and they are all listed above.");
    ImGui::Text("The solution to the digits problem in binary is very similar to the solution in decimal: we simply replace 10 in the solution with 2.");
    ImGui::Text("The number of possibile numbers that can be generated with X amount of digits in the binary number system is 2 to the X.");
    ImGui::Text("This can easily be tested by adding another digit. For each of the numbers in the list of 8 above, it will be preceded by either a 0 or a 1.");
    ImGui::Text("Because of this, each number now has two copies: one with a leading 0 and one with a leading 1. Therefore, the list doubles in size");
    ImGui::Text("meaning twice as many numbers can be generated in 4 digits as opposed to 3.");
    ImGui::Text(" ");
    ImGui::Text("With that said, we can apply this knowledge to the network portion and host portion of a subnet mask.");
    ImGui::Text("Keep in mind that a bit is a binary digit. For our 24-bit network portion and 8-bit host portion example, we can think of the network portion");
    ImGui::Text("As a 24-digit binary number and the host portion as an 8-bit binary number.");
    ImGui::Text("Now we have two new questions: how many possibilities are there for a 24-digit binary number? Well, based on our binary solution above,");
    ImGui::Text("The answer is 2 to the 24, which is about 16 million. For an 8-digit binary number, the solution is 2 to the 8, or 256.");
    ImGui::Text("What do these solutions mean? Well, let's put it this way:");
    ImGui::Text("   If 16 million possibilities exist in the 24-bit network portion of the subnet mask, that means that subnet mask supports 16 million different");
    ImGui::Text("   networks. Similarly, the 8-bit host portion of that subnet mask has 256 possibilities. That means on each network, there are 256 possible hosts.");
    ImGui::Text(" ");
    ImGui::Text("It is more important to understand this concept in terms of the host portion. Keep in mind that the subnet mask only tells us how many digits the");
    ImGui::Text("network portion is, not exactly what the number, called the network ID, actually is. We can set it to some number, and for the sake of");
    ImGui::Text("simplicity, let's set it to 000000000000000000000000 assuming the subnet mask has 24 network bits.");
    ImGui::Text(" ");
    ImGui::Text("The network portion, after being set, is then applied to IP addresses to say they are part of a network. So, any IP address using the 255.255.255.0");
    ImGui::Text("subnet mask with the first 24 bits of the IP address set to 0 will all be on the same network. But if any of those first 24 bits are not 0, the IP will");
    ImGui::Text("be on a different network. Think of it like this. 24 Grove Street is a different location than 24 Main Street. Changing the network ID, which is the street");
    ImGui::Text("name, changes what group a device belongs to, therefore what network that device belongs to.");
    ImGui::Text(" ");
    ImGui::Text("The distinction between the network portion and network ID is that the subnet mask does not determine exactly what the street name could be in our analogy;");
    ImGui::Text("What it does determine, however, is how long the street name is allowed to be. A subnet mask with 3 network bits would be analogous to only allowing a street name to be 3");
    ImGui::Text("Letters long. But that means the street name could be \"abc\". So the network portion of the subnet mask would be three \"letters\", but the network ID of our");
    ImGui::Text("network specifically would be \"abc\".");
    ImGui::Text(" ");
    ImGui::Text("Since we have set the network ID to be a bunch of zeroes, we can assume that any address we make with that network ID will be on that network.");
    ImGui::Text("24 Grove Street, 80 Grove Street, and 10 Grove Street are all on the same street, even if the house number is different. The same applies for host portions and host IDs.");
    ImGui::Text("For 24 Grove Street, the host ID would be 24 and the network ID would be Grove Street. If we say the host portion is 2 digits, that would be how long the host ID is allowed");
    ImGui::Text("to be, not what the actual value of the host ID is. The distinction between portions and IDs is the same for the host and network aspect of IP subnets.");
    ImGui::Text(" ");
    ImGui::Text("In the 255.255.255.0 example, the host portion is again 8 bits. That means any host ID for an IP address using that subnet mask will always be 8 bits long.");
    ImGui::Text("The network ID will also be 24 bits long. With all of that information combined, we get the following result:");
    ImGui::Text("   The IP address 192.168.1.1 can be written in binary as \"11000000.10101000.00000001.00000001\".");
    ImGui::Text("   The network ID of this IP address assuming we are using a 255.255.255.0 subnet mask is the first 24 bits, or \"11000000.10101000.00000001\".");
    ImGui::Text("   The host ID of this IP address assuming the same subnet mask is the last 8 bits of the address, or \"00000001\".");
    ImGui::Text("   Because the host portion is 8 bits long, we can assume that 2 to the 8, or 256, different host IDs can be generated in this network.");
    ImGui::Text("   This means that this IP address is part of the network \"11000000.10101000.00000001\" that can have 256 addresses on it.");
    ImGui::Text(" ");
    ImGui::Text("It might be obvious by now that writing a network ID in a ton of binary digits is not ideal. There is a solution to this problem: turn it into an IP address!");
    ImGui::Text("Network IDs can be written as an IP address by adding them to a host ID of 0. In other words, \"11000000.10101000.00000001\" can be written with the missing 8 bits being zeroes, or");
    ImGui::Text("\"11000000.10101000.00000001.00000000\". This is then converted into the regular IP address format: 192.168.1.0.");
    ImGui::Text("There is a caveat to allowing this, though: since the network ID is now an IP address, that IP address cannot be assigned to a device. After all,");
    ImGui::Text("it does not make sense to have a device have the ID of the entire network as its IP address!");
    ImGui::Text(" ");
    ImGui::Text("Another host ID that cannot be handed out to devices is all ones. In the case of an 8 bit host ID, that would be \"11111111\".");
    ImGui::Text("This host ID represents a broadcast, or a message that should be sent out to every member of a network. Again, it would not make sense to give such an ID to one device.");
    ImGui::Text("The broadcast host ID can also be written as an IP address on the network in the same way the network ID can be.");
    ImGui::Text("This time, it would be \"11000000.10101000.00000001.11111111\", or 192.168.1.255 in normal IPv4 format.");
    ImGui::Text(" ");
    ImGui::Text("Fortunately, the network and broadcast ID, or rather the network and broadcast IP address of a network, are the only limitations on what IP addresses can be handed out to devices on a network.");
    ImGui::Text("The IP addresses 192.168.1.1 through 192.168.1.254 are all valid addresses for devices on the 192.168.0.0 network with the subnet mask 255.255.255.0.");
    ImGui::Text(" ");
    ImGui::Text("With that said, there is one final shortcut we can make. We can write subnet masks really short. Since we know that the host bits and network bits of a subnet mask must add up to 32 bits");
    ImGui::Text("because a subnet mask is a 32-bit number, we can simply write out a slash followed by the number of network bits.");
    ImGui::Text("For the 255.255.255.0 subnet mask with 24 network bits, we can write it as /24. Therefore, the network we have currently is 192.168.0.0/24.");
    ImGui::Text("This / and network bits notation is called CIDR notation.");
    ImGui::Text("Again, the reason this works is that we know the total length of any subnet mask is 32 bits. We can subtract 24 from 32 to deduce that the host portion of the /24 mask is 8 bits.");
    ImGui::Text(" ");
    ImGui::Text("There are some other quick notes to make about how subnet masks work for networks. The \"subnet\" in subnet mask means network. A subnet is a network.");
    ImGui::Text("As a final note, let's do another example of an IP address, which resides on a subnet, and that subnet's corresponding subnet mask:"); 
    ImGui::Text("   Our IP address is 172.16.82.12 with the subnet mask /16.");
    ImGui::Text("   This subnet has a host portion of 16 bits and a network portion of 16 bits.");
    ImGui::Text("   Therefore, the subnet that 172.16.82.12 resides on contains 2 to the 16 IP addresses, or 65536 addresses.");
    ImGui::Text("   The network address of this network would be where the host portion, the last 16 bits, are all zeroes, which would be 172.16.0.0.");
    ImGui::Text("   The broacast address of this network would be where the host portion is all ones, which would be 172.16.255.255.");
    ImGui::Text("   This subnet would therefore consist of devices with addresses ranging from 172.16.0.1 to 172.16.255.254.");
    ImGui::Text(" ");
    ImGui::Text("If you can follow along with this explanation and do the calculations yourself, congratulations! You now know how to subnet.");
    ImGui::Text(" ");
    ImGui::EndChild();
    ImGui::End();
}

// ----------------------------------------------------------------------------------------------
// SECTION: Entry Point Functions
// ----------------------------------------------------------------------------------------------

int Main() { // the pseudo-main function that gets called either by WinMain() or main()
    srand((unsigned int)time(NULL)); // initialize random number generator's counter
    ImGuiInit(); // window creation and context initialization
    windowsAreOpen[0] = true;
    for (unsigned long i=1; i<sizeof(windowsAreOpen); i++) { // set non-main windows that exist to be closed; only have main window open.
        windowsAreOpen[i] = false;
    }
    currentSubnetQuestion = randomSubnetQuestion();
    currentClassQuestion = randomClassQuestion();
    while (!glfwWindowShouldClose(windowBackend)) { // main event loop
        startImGuiFrame();
        if (!windowsAreOpen[0]) { // closing main window closes entire program
            windowTerminate();
        }
        mainWindow(); // render the main window
        if (windowsAreOpen[1]) {
            exportWindow();
        }
        if (windowsAreOpen[2]) {
            debugWindow();
        }
        if (windowsAreOpen[3]) {
            studyWindow();
        }
        if (windowsAreOpen[4]) {
            IPv6Window();
        }
        if (windowsAreOpen[5]) {
            IPClassWindow();
        }
        if (windowsAreOpen[6]) {
            IPv6InfoWindow();
        }
        if (windowsAreOpen[7]) {
            IPClassInfoWindow();
        }
        if (windowsAreOpen[8]) {
            IPInfoWindow();
        }
        if (windowsAreOpen[9]) {
            subnetMaskInfoWindow();
        }
        if (graphData && subnettingStarted && subnettingSuccessful) {
            plotSubnetCubes();
        }
        endImGuiFrame();
        frameCount += 1;
        if (frameCount % 25 == 0) { // cycle the dot counter from one to three every 25 frames.
            dotCounter++;
        }
        if (dotCounter > 3ULL) { // reset the dot counter when it is more than 3.
            dotCounter = 1;
        }
    }
    windowTerminate();
    return 0;
}

#ifdef _WIN32 // call WinMain() as the main function on Windows and main() on other platforms
void WinMain() {
    Main();
}
#endif
#if defined __linux__ || __APPLE__
int main() {
    Main();
    return 0;
}
#endif
