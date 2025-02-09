#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot3d.h"
#include "implot3d_internal.h"
#include "random.h"
#include "subnetterplusplus.h"
#include <iostream>
#include <string>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <thread>
#include <mutex>
using namespace std;

typedef struct question {
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
bool aWindowIsOpen = true;
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
question currentQuestion = {"", "", "", "", "", ""}; // the question currently displayed on the screen

//  export input
char *exportInputBuffer = (char *)calloc(512, 1); // main input for export path
ofstream exportFileStream;
bool exportThreadComplete = false;
mutex exportThreadMutex;
bool currentlyInExportThread = false;

// conditionals
bool subnettingStarted = false;
bool currentExportSuccessful = false;
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
    ImGui::Text(stringArg);
    return true;
}

int argumentCallback(ImGuiInputTextCallbackData *data) {
    currentIP.IPAddress.IP32 = 0;
    totalAddedToIP = 256;
    return 1;
}

int questionChangedCallback(ImGuiInputTextCallbackData *data) {
    currentQuestionAnswered = false;
    return 1;
}

int exportChangedCallback(ImGuiInputTextCallbackData *data) {
    exportButtonPreviouslyPressed = false;
    return 1;
}

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
    for (int i=0; i<debugEntries.size(); i++) {
        ImGui::Text((">>> " + debugEntries[i]).c_str());
    }
}

// TODO: test this function
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
        ImGui::Text(currentQuestion.questionString.c_str());
        ImGui::InputText("Subnet Mask", studyInputBuffer1, 255, ImGuiInputTextFlags_CallbackEdit, &questionChangedCallback);
        ImGui::InputText("Block Size", studyInputBuffer2, 255, ImGuiInputTextFlags_CallbackEdit, &questionChangedCallback);
        ImGui::InputText("Network Address", studyInputBuffer3, 255, ImGuiInputTextFlags_CallbackEdit, &questionChangedCallback);
        ImGui::InputText("First Usable Address in Subnet", studyInputBuffer4, 255, ImGuiInputTextFlags_CallbackEdit, &questionChangedCallback);
        ImGui::InputText("Last Usable Address in Subnet", studyInputBuffer5, 255, ImGuiInputTextFlags_CallbackEdit, &questionChangedCallback);
        ImGui::InputText("Broadcast Address", studyInputBuffer6, 255, ImGuiInputTextFlags_CallbackEdit, &questionChangedCallback);
        ImGui::InputText("Binary for Provided IP", studyInputBuffer7, 255, ImGuiInputTextFlags_CallbackEdit, &questionChangedCallback);
        ImGui::InputText("Binary for Subnet Mask", studyInputBuffer8, 255, ImGuiInputTextFlags_CallbackEdit, &questionChangedCallback);
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
    ImGui::BeginMenuBar();
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
    ImGui::InputText("IP", mainInputBuffer1, 255, ImGuiInputTextFlags_CallbackEdit, &argumentCallback);
    ImGui::InputText("Netmask 1", mainInputBuffer2, 255, ImGuiInputTextFlags_CallbackEdit, &argumentCallback);
    ImGui::InputText("Netmask 2", mainInputBuffer3, 255, ImGuiInputTextFlags_CallbackEdit, &argumentCallback);
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
        ImGui::Text(("Number of Debug Entries: " + to_string(debugEntries.size())).c_str());
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
    if (ImGui::Button("Go to End") && totalAddedToIP != totalSubnetsToGenerate) {
        currentIP += (unsigned int)(totalSubnetsToGenerate - totalAddedToIP - 256) * (unsigned int)netMaskArg2.blockSize;
        totalAddedToIP = totalSubnetsToGenerate - (totalSubnetsToGenerate % 256);
        if (totalAddedToIP < 256) {
            totalAddedToIP = 256;
            currentIP = (totalSubnetsToGenerate - 256) * netMaskArg2.blockSize;
        }
    }
    if (subnettingStarted) {
        ImGui::BeginChild("ScrollWheel");
        SubnetMask swapMask;
        IPArg = IP(mainInputBuffer1);
        netMaskArg1 = SubnetMask(mainInputBuffer2);
        netMaskArg2 = SubnetMask(mainInputBuffer3);
        if (netMaskArg1.networkBits > netMaskArg2.networkBits) {
            swapMask = netMaskArg1;
            netMaskArg1 = netMaskArg2;
            netMaskArg2 = swapMask;
        }
        networkMagnitudeDifference = netMaskArg1.hostBits - netMaskArg2.hostBits;
        totalSubnetsToGenerate = 1ULL<<networkMagnitudeDifference;
        if (debugFlag) {
            ImGui::Text(("Added total in main: " + to_string(totalAddedToIP)).c_str()); 
            ImGui::Text(("Subnets to generate: " + to_string(totalSubnetsToGenerate)).c_str()); 
            ImGui::Text(("Magnitude difference: " + to_string(networkMagnitudeDifference)).c_str());
            ImGui::Text(("Current IP: " + currentIP.IPString).c_str());
        }
        if (currentIP.IPAddress.IP32 == 0) {
            currentIP = IPArg & netMaskArg1;
        }
        timedVLSM(currentIP, netMaskArg1, netMaskArg2, false);
        ImGui::EndChild();
    }
    ImGui::End();
}

void exportWindow() {
    ImGui::Begin("Export...", windowsAreOpen+1);
    ImGui::InputText("Export Path", exportInputBuffer, 255, ImGuiInputTextFlags_CallbackEdit, &exportChangedCallback);
    if (ImGui::Button("Export")) {
        if (currentlyInExportThread) {
            ImGui::End();
            return;
        }
        unique_lock exportThreadLock(exportThreadMutex);
        exportThreadComplete = false;
        exportThreadLock.unlock();
        currentExportSuccessful = false;
        exportButtonPreviouslyPressed = true;
        IPArg = IP(mainInputBuffer1);
        netMaskArg1 = SubnetMask(mainInputBuffer2);
        netMaskArg2 = SubnetMask(mainInputBuffer3);
        if (currentIP.IPAddress.IP32 == 0) {
            currentIP = IPArg & netMaskArg2;
        }
        string exportString = string(exportInputBuffer);
        debugLog("Export Button pressed with path " + exportString);
        string mainPath = exportString.substr(0, exportString.find_last_of("\\/") + 1);
        string resultFile = exportString.substr(exportString.find_last_of("\\/") + 1, exportString.length() - 1);
        if (filesystem::exists(filesystem::path(mainPath)) && resultFile.contains(".txt")) {
            debugLog("Entered path exists");
            debugLog("Main Path: " + mainPath);
            debugLog("Result File: " + resultFile);
            exportFileStream.open(exportString);
            debugLog("File created!");
            thread exportThread = thread([]() -> void {currentlyInExportThread = true; timedVLSM(IPArg, netMaskArg1, netMaskArg2, true); unique_lock exportThreadLock(exportThreadMutex); exportThreadComplete = true; currentlyInExportThread = false;});
            exportThread.detach();
            debugLog("File exported!");
            currentExportSuccessful = true;
        } else {
            currentExportSuccessful = false;
            debugLog("Main Path: " + mainPath);
            debugLog("Result File: " + resultFile);
        }
    }
    if (!exportButtonPreviouslyPressed) {
        ImGui::End();
        return;
    }
    if (currentExportSuccessful) {
        unique_lock exportThreadLock(exportThreadMutex);
        if (!exportThreadComplete) {
            ImGui::Text(("Exporting" + string(dotCounter, '.')).c_str());
        }
        else {ImGui::Text("File exported!");}
    } else {
        ImGui::Text("File could not be exported. Is the path correct?");
    }
    ImGui::End();
    return;
}

int Main() {
    // window creation and context initialization
    srand((unsigned int)time(NULL));
    plotBackGroundColor = ImVec4(0.1137f, 0.1843f, 0.2863f, 1.0f);
    ImGuiInit();
    // float x, y;
    // glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &x, &y);
    // ImGui::GetStyle().ScaleAllSizes(x);
    windowsAreOpen[0] = true;
    for (int i=1; i<sizeof(windowsAreOpen); i++) { // set windows that exist to false; only have main window open.
        windowsAreOpen[i] = false;
    }
    currentQuestion = randomQuestion();
    // debug();
    while (!glfwWindowShouldClose(windowBackend)) { // main event loop
        startImGuiFrame();
        if (!windowsAreOpen[0]) { // closing main window closes entire program
            windowTerminate();
        }
        mainWindow();
        if (windowsAreOpen[1]) {
            exportWindow();
        }
        if (windowsAreOpen[2]) {
            ImGui::Begin("Debug Log", windowsAreOpen+2);
            ImGui::Text(("Frame Count: " + to_string(frameCount)).c_str());
            ImGui::Text(("Dot Count: " + to_string(dotCounter)).c_str());
            ImGui::BeginChild("ScrollWheel");
            printDebugEntries();
            ImGui::EndChild();
            ImGui::End();
        }
        if (windowsAreOpen[3]) {
            studyWindow();
        }
        if (graphData && subnettingStarted) {
            plotSubnetCubes();
        }
        for (int i=0; i<sizeof(windowsAreOpen); i++) { // TODO: change how open windows are handled.
            aWindowIsOpen = aWindowIsOpen || windowsAreOpen[i];
        }
        if (!aWindowIsOpen) {
            windowTerminate();
        }
        aWindowIsOpen = false;
        endImGuiFrame();
        frameCount += 1;
        if (frameCount % 25 == 0) {
            dotCounter++;
        }
        if (dotCounter > 3ULL) {
            dotCounter = 1;
        }
    }
    return 0;
}

#ifdef _WIN32
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