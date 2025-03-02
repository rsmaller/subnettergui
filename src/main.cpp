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
#include <iostream>
#include <string>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <thread>
#include <mutex>
using namespace std;

typedef struct question {
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
} question;

GLFWwindow *windowBackend = nullptr; // invisible GLFW window that allows ImGui to appear. currently the GLFW window itself cannot render new frames.

const int totalNumberOfWindows = 4; // control variables to properly exit the program when windows are closed.
bool windowsAreOpen[totalNumberOfWindows];
//  Indices:
//  0 - Main Window
//  1 - Export Window
//  2 - Debug Window
//  3 - Study Window
ImVec4 plotBackGroundColor;
unsigned long long int frameCount = 0;
unsigned long long int dotCounter = 1;

//  buffers for input data
char *mainInputBuffer1 = (char *)calloc(256, 1); // main IP input
char *mainInputBuffer2 = (char *)calloc(256, 1); // netmask 1 input
char *mainInputBuffer3 = (char *)calloc(256, 1); // netmask 2 input

//  buffers for study input data
char *studyInputBuffer1 = (char *)calloc(256, 1); // main study network IP input
char *studyInputBuffer2 = (char *)calloc(256, 1); // reserved input
char *studyInputBuffer3 = (char *)calloc(256, 1); // reserved input
char *studyInputBuffer4 = (char *)calloc(256, 1); // reserved input
char *studyInputBuffer5 = (char *)calloc(256, 1); // reserved input
char *studyInputBuffer6 = (char *)calloc(256, 1); // reserved input
char *studyInputBuffer7 = (char *)calloc(256, 1); // reserved input
char *studyInputBuffer8 = (char *)calloc(256, 1); // reserved input
question currentQuestion = {IP(0), SubnetMask(0), "", "", "", "", "", ""}; // the question currently displayed on the screen

//  export input
char *exportInputBuffer = (char *)calloc(512, 1); // main input for export path
extern ofstream exportFileStream;
bool exportThreadComplete = false;
mutex exportThreadMutex;
bool currentlyInExportThread = false;

// conditionals
bool subnettingStarted = false;
extern bool subnettingSuccessful;
bool exportButtonPreviouslyPressed = false;
bool graphData = false;
bool nextButtonShown = false;
bool prevButtonShown = false;
bool currentQuestionAnswered = false;
int networkMagnitudeDifference = 0;
unsigned long long int totalSubnetsToGenerate = 0;

IP currentIP(0); // IP cursor variables
// subnet and cube prototypes
IP IPArg;
SubnetMask netMaskArg1;
SubnetMask netMaskArg2;
extern unsigned long long totalAddedToIP;

vector<string> debugEntries;

void debugLog(string lineToAdd) {
    debugEntries.push_back(lineToAdd);
}

void windowTerminate() {
    glfwTerminate();
    exit(0);
}

bool sameLineInIf() {
    ImGui::SameLine();
    return true;
}

bool textInIf(const char *stringArg) {
    ImGui::Text("%s", stringArg);
    return true;
}

int argumentResetFunc() {
    currentIP.IPAddress.IP32 = 0;
    totalAddedToIP = 256;
    return 1;
}

int questionResetFunc() {
    currentQuestionAnswered = false;
    return 1;
}

int exportResetFunc() {
    exportButtonPreviouslyPressed = false;
    return 1;
}

int (*argumentChangedCallback)(ImGuiInputTextCallbackData *data) = (int (*)(ImGuiInputTextCallbackData *))&argumentResetFunc;
int (*questionChangedCallback)(ImGuiInputTextCallbackData *data) = (int (*)(ImGuiInputTextCallbackData *))&questionResetFunc;
int (*exportChangedCallback)(ImGuiInputTextCallbackData *data) = (int (*)(ImGuiInputTextCallbackData *))&exportResetFunc;

void glfwErrorCallback(int error, const char *msg) {
    if (error != 65544) printf("Error %d: %s\n", error, msg);
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
    windowBackend = glfwCreateWindow(100, 100, "backend", NULL, NULL);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwMakeContextCurrent(windowBackend);
    float x, y;
    glfwGetMonitorContentScale(monitor, &x, &y);
    glfwHideWindow(windowBackend);
    glewInit();
    ImGui::SetCurrentContext(ImGui::CreateContext());
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
    primaryViewPort -> Size = ImVec2(1000, 800);
    ImPlot3D::CreateContext();
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        debugLog("OpenGL error in ImGuiInit():" + to_string(error));
        printf("OpenGL Error!: %d\n", error);
    }
    ImGui::SetNextWindowViewport(ImGui::GetCurrentContext()->Viewports[0]->ID); // detach viewport from window
}

void startImGuiFrame() {
    glfwMakeContextCurrent(windowBackend);
    glfwPollEvents(); // grabs events every loop. prevents window from not responding
    glClearColor(0.0549019607843f, 0.0549019607843f, 0.0549019607843f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
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

void printDebugEntries() {
    for (unsigned long i=0; i<debugEntries.size(); i++) {
        ImGui::Text("%s", (">>> " + debugEntries[i]).c_str());
    }
}

IP constructRandomIP() {
    return IP(getRandomIPNumber());
}

SubnetMask constructRandomSubnetMask() {
    return SubnetMask(getRandomInteger(0, 32));
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
    ImGui::PushStyleColor(ImGuiCol_WindowBg, plotBackGroundColor);
    ImGui::Begin("3D Subnet Plot", &graphData);
    ImGui::PopStyleColor();
    if (ImPlot3D::BeginPlot(("Big Subnet Cube Side Length: " + to_string(cube1SideLength) + "\nSmall Subnet Cube Side Length: " + to_string(cube2SideLength) + "\n" + to_string(netMaskArg1.blockSize / netMaskArg2.blockSize) + " Small Cubes can fit inside the Big Cube").c_str())) {
        ImPlot3D::SetupAxes("x", "y", "z");
        ImPlot3D::SetupAxesLimits(-vertex1DistanceFromOrigin, vertex1DistanceFromOrigin, -vertex1DistanceFromOrigin, vertex1DistanceFromOrigin, -vertex1DistanceFromOrigin, vertex1DistanceFromOrigin, ImPlot3DCond_Always);
        ImPlot3D::PlotMesh(("Big Subnet [/" + to_string(netMaskArg1.networkBits) + "]").c_str(), big_cube_vtx, ImPlot3D::cube_idx, ImPlot3D::CUBE_VTX_COUNT, ImPlot3D::CUBE_IDX_COUNT);
        ImPlot3D::PlotMesh(("Small Subnet [/" + to_string(netMaskArg2.networkBits) + "]").c_str(), small_cube_vtx, ImPlot3D::cube_idx, ImPlot3D::CUBE_VTX_COUNT, ImPlot3D::CUBE_IDX_COUNT);
        ImPlot3D::EndPlot();
    }
    ImGui::End();
}

question randomQuestion() {
    question returnValue;
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

void debug() {
    debugLog("debugging function ran");
    return;
}

void resetCurrentQuestion() {
    memcpy(studyInputBuffer1, "", 255);
    memcpy(studyInputBuffer2, "", 255);
    memcpy(studyInputBuffer3, "", 255);
    memcpy(studyInputBuffer4, "", 255);
    memcpy(studyInputBuffer5, "", 255);
    memcpy(studyInputBuffer6, "", 255);
    memcpy(studyInputBuffer7, "", 255);
    memcpy(studyInputBuffer8, "", 255);
    currentQuestionAnswered = false;
}

void showAnswers() { 
    memcpy(studyInputBuffer1, currentQuestion.answer1.c_str(), 255);
    memcpy(studyInputBuffer2, currentQuestion.answer2.c_str(), 255);
    memcpy(studyInputBuffer3, currentQuestion.answer3.c_str(), 255);
    memcpy(studyInputBuffer4, currentQuestion.answer4.c_str(), 255);
    memcpy(studyInputBuffer5, currentQuestion.answer5.c_str(), 255);
    memcpy(studyInputBuffer6, currentQuestion.answer6.c_str(), 255);
    memcpy(studyInputBuffer7, currentQuestion.answer7.c_str(), 255);
    memcpy(studyInputBuffer8, currentQuestion.answer8.c_str(), 255);
    currentQuestionAnswered = false;
}

void checkAnswers() {
    currentQuestionAnswered = true;
}

void studyWindow() {
    ImGui::Begin("Study", windowsAreOpen+3);
    if (currentQuestion.questionString.compare("")) { 
        ImGui::Text("%s", currentQuestion.questionString.c_str());
        ImGui::InputText("Subnet Mask", studyInputBuffer1, 255, ImGuiInputTextFlags_CallbackEdit, questionChangedCallback);
        ImGui::InputText("Block Size", studyInputBuffer2, 255, ImGuiInputTextFlags_CallbackEdit, questionChangedCallback);
        ImGui::InputText("Network Address", studyInputBuffer3, 255, ImGuiInputTextFlags_CallbackEdit, questionChangedCallback);
        ImGui::InputText("First Usable Address in Subnet", studyInputBuffer4, 255, ImGuiInputTextFlags_CallbackEdit, questionChangedCallback);
        ImGui::InputText("Last Usable Address in Subnet", studyInputBuffer5, 255, ImGuiInputTextFlags_CallbackEdit, questionChangedCallback);
        ImGui::InputText("Broadcast Address", studyInputBuffer6, 255, ImGuiInputTextFlags_CallbackEdit, questionChangedCallback);
        ImGui::InputText(("Binary for " + currentQuestion.questionIP.IPString).c_str(), studyInputBuffer7, 255, ImGuiInputTextFlags_CallbackEdit, questionChangedCallback);
        ImGui::InputText(("Binary for /" + to_string(currentQuestion.questionNetMask.networkBits) + " Mask").c_str(), studyInputBuffer8, 255, ImGuiInputTextFlags_CallbackEdit, questionChangedCallback);
        if (ImGui::Button("Submit Answers")) {checkAnswers();}
        ImGui::SameLine();
        if (ImGui::Button("Show Answers")) {showAnswers();}
        ImGui::SameLine();
        if (ImGui::Button("Hide Answers")) {resetCurrentQuestion();}
        ImGui::SameLine();
    }
    if (ImGui::Button("Generate New Question")) {
        currentQuestion = randomQuestion();
        resetCurrentQuestion();
    }
    if (currentQuestionAnswered) {
        if (!currentQuestion.answer1.compare(studyInputBuffer1)) {ImGui::Text("Subnet Mask is correct!");} else {ImGui::Text("Subnet Mask is incorrect.");}
        if (!currentQuestion.answer2.compare(studyInputBuffer2)) {ImGui::Text("Block Size is correct!");} else {ImGui::Text("Block Size is incorrect.");}
        if (!currentQuestion.answer3.compare(studyInputBuffer3)) {ImGui::Text("Network Address is correct!");} else {ImGui::Text("Network Address is incorrect.");}
        if (!currentQuestion.answer4.compare(studyInputBuffer4)) {ImGui::Text("First Usable Address is correct!");} else {ImGui::Text("First Usable Address is incorrect.");}
        if (!currentQuestion.answer5.compare(studyInputBuffer5)) {ImGui::Text("Last Usable Address is correct!");} else {ImGui::Text("Last Usable Address is incorrect.");}
        if (!currentQuestion.answer6.compare(studyInputBuffer6)) {ImGui::Text("Broadcast Address is correct!");} else {ImGui::Text("Broadcast Address is incorrect.");}
        if (!currentQuestion.answer7.compare(studyInputBuffer7)) {ImGui::Text("Binary for Provided IP is correct!");} else {ImGui::Text("Binary for Provided IP is incorrect.");}
        if (!currentQuestion.answer8.compare(studyInputBuffer8)) {ImGui::Text("Binary for Subnet Mask is correct!");} else {ImGui::Text("Binary for Subnet Mask is incorrect.");}
    }
    ImGui::End();
    return;
}

void mainWindow() {
    ImGui::Begin("Subnetter++", windowsAreOpen+0, ImGuiWindowFlags_MenuBar);
    ImGui::BeginMenuBar(); // Indented conditionals are part of the menu bar.
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Export")) {debugLog("Export Window Opened"); windowsAreOpen[1] = true;}
            if (ImGui::MenuItem("Study")) {windowsAreOpen[3] = true;}
            if (ImGui::MenuItem("Open Debug Window")) {windowsAreOpen[2] = true;}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options")) {
            if (ImGui::MenuItem("Placeholder 1")) {debugLog("Placeholder 1 Opened");}
            if (ImGui::MenuItem("Placeholder 2")) {debugLog("Placeholder 2 Opened");}
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
    netMaskArg1 = SubnetMask(mainInputBuffer2);
    netMaskArg2 = SubnetMask(mainInputBuffer3);
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

void exportWindow() {
    ImGui::Begin("Export...", windowsAreOpen+1);
    ImGui::InputText("Export Path", exportInputBuffer, 255, ImGuiInputTextFlags_CallbackEdit, exportChangedCallback); // if this input is changed, reset the export success message.
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
    ImGui::Begin("Debug Log", windowsAreOpen+2);
    ImGui::Text("%s", ("Frame Count: " + to_string(frameCount)).c_str());
    ImGui::Text("%s", ("Dot Count: " + to_string(dotCounter)).c_str());
    ImGui::BeginChild("ScrollWheel");
    printDebugEntries();
    ImGui::EndChild();
    ImGui::End();
}

int Main() { // the pseudo-main function that gets called either by WinMain() or main()
    srand((unsigned int)time(NULL)); // initialize random number generator's counter
    plotBackGroundColor = ImVec4(0.1137f, 0.1843f, 0.2863f, 1.0f); // color for the ImPlot3D background
    ImGuiInit(); // window creation and context initialization
    windowsAreOpen[0] = true;
    for (unsigned long i=1; i<sizeof(windowsAreOpen); i++) { // set non-main windows that exist to be closed; only have main window open.
        windowsAreOpen[i] = false;
    }
    currentQuestion = randomQuestion();
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