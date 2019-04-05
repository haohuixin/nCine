#include <ncine/imgui.h>

#include "apptest_simdbench.h"
#include <ncine/Application.h>
#include <ncine/LuaStateManager.h>
#include <ncine/LuaUtils.h>
#include <ncine/IFile.h>
#include <ncine/Timer.h>
#include <ncine/Vector4.h>
#include <ncine/Quaternion.h>

namespace Names
{
	const char *testSet = "test_set";
	const char *testSystem = "system";
	const char *testName = "name";
	const char *testIterations = "iterations";
	const char *testTimings = "timings";
	const char *testTotalTime = "total_time";
	const char *testMaxTime = "max_time";
	const char *testMinTime = "min_time";
	const char *testAverage = "average";
	const char *testStdDeviation = "standard_deviation";
	const char *testRelStdDeviation = "relative_standard_deviation";
};

namespace {

enum Tests
{
	Vector4Add,
	Vector4Sub,
	Vector4Mul,
	Vector4Div,
	Vector4Length,
	Vector4SqrLength,
	Vector4Normalize,
	Vector4Dot,
	QuaternionMult,
	MatrixMult,
	MatrixTrans,
	MatrixVecMult,

	Count
};

typedef float (*TestFunction)(unsigned int);
const unsigned int MaxTestRuns = 3;
const unsigned int MaxRepetitions = 32;
const unsigned int MaxIterations = 10000000;

struct TestInfo
{
	TestInfo()
		: name(nctl::String(128)), func(nullptr), totalTime(0.0f),
		  maxTime(0.0f), minTime(0.0f), average(0.0f), stdDeviation(0.0f),
		  numRepetitions(0), numIterations(0) { }

	nctl::String name;
	TestFunction func;
	float times[MaxRepetitions];
	float totalTime;
	float maxTime;
	float minTime;
	float average;
	float stdDeviation;
	unsigned int numRepetitions;
	unsigned int numIterations;
};

struct TestRun
{
	nctl::String filename = nctl::String(128);
	nctl::String system;
	TestInfo testInfos[Tests::Count];
};

TestInfo testInfos[Tests::Count];
const char *testNames[Tests::Count];
TestRun testRuns[MaxTestRuns];
int currentTestRun = 0;
const unsigned int MaxStringLength = 128;
char loadingFilename[MaxStringLength] = "timings.lua";
char savingFilename[MaxStringLength] = "timings.lua";
bool includeStatsWhenSaving = false;

static nc::Timer timer;
float nums[MaxIterations];
nc::Vector4f vecsA[MaxIterations];
nc::Vector4f vecsB[MaxIterations];
nc::Vector4f vecsC[MaxIterations];
nc::Quaternionf quats[MaxIterations];
nc::Matrix4x4f mats[MaxIterations];

int currentTest = 0;
int numIterations = MaxIterations / 5;
int numRepetitions = MaxRepetitions / 2;

bool runningAllTests = false;
unsigned int allTestsIndex = 0;
nctl::String progressMsg;
const ImVec4 Green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
const ImVec4 Red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

const char *system()
{
#if defined(__WIN32)
	return "Windows";
#elif defined(__APPLE__)
	return "macOS";
#elif defined(__ANDROID__)
	return "Android";
#else
	return "Linux";
#endif
}

void calculateStats(TestInfo &t)
{
	t.maxTime = t.times[0];
	t.minTime = t.times[0];
	for (unsigned int i = 1; i < t.numRepetitions; i++)
	{
		if (t.times[i] > t.maxTime)
			t.maxTime = t.times[i];
		else if (t.times[i] < t.minTime)
			t.minTime = t.times[i];
	}

	t.totalTime = 0.0f;
	for (unsigned int i = 0; i < t.numRepetitions; i++)
		t.totalTime += t.times[i];
	t.average = t.totalTime / static_cast<float>(t.numRepetitions);

	t.stdDeviation = 0.0f;
	if (t.numRepetitions > 1)
	{
		for (unsigned int i = 0; i < t.numRepetitions; i++)
			t.stdDeviation += (t.times[i] - t.average) * (t.times[i] - t.average);
		t.stdDeviation /= static_cast<float>(t.numRepetitions - 1);
		t.stdDeviation = sqrtf(t.stdDeviation);
	}
}

void runTest(TestInfo &t, unsigned int numRepetitions, unsigned int numIterations)
{
	ASSERT(t.numRepetitions > 0);
	ASSERT(t.numIterations > 0);
	FATAL_ASSERT(t.func != nullptr);

	t.numRepetitions = numRepetitions;
	t.numIterations = numIterations;
	for (unsigned int i = 0; i < t.numRepetitions; i++)
		t.times[i] = t.func(t.numIterations) * 1000;

	calculateStats(t);
}

void resetVecs(unsigned int iterations)
{
	for (unsigned int i = 0 ; i < iterations; i++)
	{
		nums[i] = i;
		vecsA[i].set(i, i, i, i);
		vecsB[i].set(i, i, i, i);
		vecsC[i].set(i, i, i, i);
	}
}

void resetQuats(unsigned int iterations)
{
	for (unsigned int i = 0 ; i < iterations; i++)
		quats[i].set(i, i, i, i);
}

void resetMats(unsigned int iterations)
{
	for (unsigned int i = 0 ; i < iterations; i++)
	{
		mats[i][0].set(i, i, i, i);
		mats[i][1].set(i, i, i, i);
		mats[i][2].set(i, i, i, i);
		mats[i][3].set(i, i, i, i);
	}
}

float benchVector4Add(unsigned int iterations)
{
	resetVecs(iterations);

	timer.start();
	for (unsigned int i = 0; i < iterations - 1; i++)
		vecsC[i] = vecsA[i] + vecsB[i];

	return timer.interval();
}

float benchVector4Sub(unsigned int iterations)
{
	resetVecs(iterations);

	timer.start();
	for (unsigned int i = 0; i < iterations - 1; i++)
		vecsC[i] = vecsA[i] - vecsB[i];

	return timer.interval();
}

float benchVector4Mul(unsigned int iterations)
{
	resetVecs(iterations);

	timer.start();
	for (unsigned int i = 0; i < iterations - 1; i++)
		vecsC[i] = vecsA[i] * vecsB[i];

	return timer.interval();
}

float benchVector4Div(unsigned int iterations)
{
	resetVecs(iterations);

	timer.start();
	for (unsigned int i = 0; i < iterations - 1; i++)
		vecsC[i] = vecsA[i] / vecsB[i];

	return timer.interval();
}

float benchVector4Length(unsigned int iterations)
{
	resetVecs(iterations);

	timer.start();
	for (unsigned int i = 0; i < iterations; i++)
		nums[i] = vecsA[i].length();

	return timer.interval();
}

float benchVector4SqrLength(unsigned int iterations)
{
	resetVecs(iterations);

	timer.start();
	for (unsigned int i = 0; i < iterations; i++)
		nums[i] = vecsA[i].sqrLength();

	return timer.interval();
}

float benchVector4Normalize(unsigned int iterations)
{
	resetVecs(iterations);

	timer.start();
	for (unsigned int i = 0; i < iterations; i++)
		vecsC[i] = vecsA[i].normalize();

	return timer.interval();
}

float benchVector4Dot(unsigned int iterations)
{
	resetVecs(iterations);

	timer.start();
	for (unsigned int i = 0; i < iterations - 1; i++)
		nums[i] = nc::dot(vecsA[i], vecsB[i]);

	return timer.interval();
}

float benchQuaternionMult(unsigned int iterations)
{
	resetQuats(iterations);

	timer.start();
	for (unsigned int i = 0; i < iterations - 1; i++)
		quats[i] = quats[i] * quats[i+1];

	return timer.interval();
}

float benchMatrixMult(unsigned int iterations)
{
	resetMats(iterations);

	timer.start();
	for (unsigned int i = 0; i < iterations - 1; i++)
		mats[i] = mats[i] * mats[i+1];

	return timer.interval();
}

float benchMatrixTrans(unsigned int iterations)
{
	resetMats(iterations);

	timer.start();
	for (unsigned int i = 0; i < iterations; i++)
		mats[i] = mats[i].transpose();

	return timer.interval();
}

float benchMatrixVecMult(unsigned int iterations)
{
	resetVecs(iterations);
	resetMats(iterations);

	timer.start();
	for (unsigned int i = 0; i < iterations; i++)
		vecsC[i] = mats[i] * vecsA[i];

	return timer.interval();
}

void indent(nctl::String &string, int amount)
{
	FATAL_ASSERT(amount >= 0);
	for (int i = 0; i < amount; i++)
		string.append("\t");
}

bool loadTestRun(const char *filename, unsigned int index)
{
	nc::LuaStateManager luaState(
		nc::LuaStateManager::ApiType::NONE,
		nc::LuaStateManager::StatisticsTracking::DISABLED,
		nc::LuaStateManager::StandardLibraries::NOT_LOADED);

	if (luaState.run(filename) == false)
	{
		LOGW_X("Cannot run script \"%s\" for index %u", filename, index);
		return false;
	}
	lua_State *L = luaState.state();

	testRuns[index].filename = filename;
	nc::LuaUtils::retrieveGlobalTable(L, Names::testSet);
	testRuns[index].system = nc::LuaUtils::retrieveField<const char *>(L, -1, Names::testSystem);

	const unsigned int numTests = nc::LuaUtils::rawLen(L, -1);
	for (unsigned int testIndex = 0; testIndex < numTests; testIndex++)
	{
		TestInfo &t = testRuns[index].testInfos[testIndex];
		nc::LuaUtils::rawGeti(L, -1, testIndex + 1); // Lua arrays start from index 1

		t.numIterations = nc::LuaUtils::retrieveField<uint32_t>(L, -1, Names::testIterations);
		t.name = nc::LuaUtils::retrieveField<const char *>(L, -1, Names::testName);

		nc::LuaUtils::retrieveFieldTable(L, -1, Names::testTimings);
		t.numRepetitions = nc::LuaUtils::rawLen(L, -1);
		for (unsigned int repIndex = 0; repIndex < t.numRepetitions; repIndex++)
		{
			nc::LuaUtils::rawGeti(L, -1, repIndex + 1);
			t.times[repIndex] = nc::LuaUtils::retrieve<float>(L, -1);
			nc::LuaUtils::pop(L);
		}
		nc::LuaUtils::pop(L);

		nc::LuaUtils::pop(L);
		calculateStats(t);
	}

	nc::LuaUtils::pop(L);

	return true;
}

void saveTestRun(const char *filename, bool includeStatistics)
{
	nc::LuaStateManager luaState(
		nc::LuaStateManager::ApiType::NONE,
		nc::LuaStateManager::StatisticsTracking::DISABLED,
		nc::LuaStateManager::StandardLibraries::NOT_LOADED);

	nctl::String file(8192);
	int amount = 0;

	indent(file, amount); file.formatAppend("%s = {\n", Names::testSet);

	amount++;
	indent(file, amount); file.formatAppend("%s = \"%s\",\n", Names::testSystem, system());

	for (unsigned int testIndex = 0; testIndex < Tests::Count; testIndex++)
	{
		if (testInfos[testIndex].numRepetitions == 0)
			continue;

		const TestInfo &t = testInfos[testIndex];
		indent(file, amount); file.append("{\n");

		amount++;
		indent(file, amount); file.formatAppend("%s = \"%s\",\n", Names::testName, t.name.data());
		indent(file, amount); file.formatAppend("%s = %d,\n", Names::testIterations, t.numIterations);
		indent(file, amount); file.formatAppend("%s = {", Names::testTimings);
		for (unsigned int i = 0; i < t.numRepetitions; i++)
			file.formatAppend(" %f%s", t.times[i], (i < t.numRepetitions - 1) ? "," : " ");
		file.formatAppend("}%s\n", includeStatistics ? "," : "");

		if (includeStatistics)
		{
			indent(file, amount); file.formatAppend("%s = %f,\n", Names::testTotalTime, t.totalTime);
			indent(file, amount); file.formatAppend("%s = %f,\n", Names::testMaxTime, t.maxTime);
			indent(file, amount); file.formatAppend("%s = %f,\n", Names::testMinTime, t.minTime);
			indent(file, amount); file.formatAppend("%s = %f,\n", Names::testAverage, t.average);
			indent(file, amount); file.formatAppend("%s = %f,\n", Names::testStdDeviation, t.stdDeviation);
			indent(file, amount); file.formatAppend("%s = %f\n", Names::testRelStdDeviation, t.average > 0.0f ? 100.0f * t.stdDeviation / t.average : 0.0f);
		}

		amount--;
		indent(file, amount); file.formatAppend("}%s\n", (testIndex < Tests::Count - 1) ? "," : "");
	}

	amount--;
	indent(file, amount); file.append("}\n");

	nctl::UniquePtr<nc::IFile> fileHandle = nc::IFile::createFileHandle(filename);
	fileHandle->open(nc::IFile::OpenMode::WRITE | nc::IFile::OpenMode::BINARY);
	fileHandle->write(file.data(), file.length());
	fileHandle->close();
}

}

nc::IAppEventHandler *createAppEventHandler()
{
	return new MyEventHandler;
}

void MyEventHandler::onPreInit(nc::AppConfiguration &config)
{
#ifdef __ANDROID__
	const char *extStorage = getenv("EXTERNAL_STORAGE");
	nctl::String dataPath;
	dataPath = extStorage ? extStorage : "/sdcard";
	dataPath += "/ncine/";
	config.setDataPath(dataPath);
#endif

	config.enableAudio(false);
	config.enableInfoText(false);
	config.enableProfilerGraphs(false);
	config.enableThreads(false);
}

void MyEventHandler::onInit()
{
	ImGuiIO& io = ImGui::GetIO();
#ifdef __ANDROID__
	io.FontGlobalScale = 3.0f;
#endif

	testInfos[Tests::Vector4Add].func = benchVector4Add;
	testInfos[Tests::Vector4Add].name = "Vector4 Add";
	testInfos[Tests::Vector4Sub].func = benchVector4Sub;
	testInfos[Tests::Vector4Sub].name = "Vector4 Sub";
	testInfos[Tests::Vector4Mul].func = benchVector4Mul;
	testInfos[Tests::Vector4Mul].name = "Vector4 Mul";
	testInfos[Tests::Vector4Div].func = benchVector4Div;
	testInfos[Tests::Vector4Div].name = "Vector4 Div";

	testInfos[Tests::Vector4Length].func = benchVector4Length;
	testInfos[Tests::Vector4Length].name = "Vector4 Length";
	testInfos[Tests::Vector4SqrLength].func = benchVector4SqrLength;
	testInfos[Tests::Vector4SqrLength].name = "Vector4 SqrLength";
	testInfos[Tests::Vector4Normalize].func = benchVector4Normalize;
	testInfos[Tests::Vector4Normalize].name = "Vector4 Normalize";
	testInfos[Tests::Vector4Dot].func = benchVector4Dot;
	testInfos[Tests::Vector4Dot].name = "Vector4 Dot";

	testInfos[Tests::QuaternionMult].func = benchQuaternionMult;
	testInfos[Tests::QuaternionMult].name = "Quaternion Mul";

	testInfos[Tests::MatrixMult].func = benchMatrixMult;
	testInfos[Tests::MatrixMult].name = "Matrix Mul";
	testInfos[Tests::MatrixTrans].func = benchMatrixTrans;
	testInfos[Tests::MatrixTrans].name = "Matrix Trans";
	testInfos[Tests::MatrixVecMult].func = benchMatrixVecMult;
	testInfos[Tests::MatrixVecMult].name = "MatrixVec Mult";

	for (unsigned int i = 0; i < Tests::Count; i++)
		testNames[i] = testInfos[i].name.data();
}

void MyEventHandler::onFrameStart()
{
	if (runningAllTests)
	{
		if (allTestsIndex >= Tests::Count)
		{
			allTestsIndex = 0;
			runningAllTests = false;
		}
		else
		{
			runTest(testInfos[allTestsIndex], numRepetitions, numIterations);
			allTestsIndex++;
		}
	}

	TestInfo &t = testInfos[currentTest];

	ImGui::Begin("Benchmark");
	if (ImGui::CollapsingHeader("Load Test Runs"))
	{
		if (ImGui::ArrowButton("##LeftTestRun", ImGuiDir_Left))
			currentTestRun--;
		ImGui::SameLine();
		if (ImGui::ArrowButton("##RightTestRun", ImGuiDir_Right))
			currentTestRun++;
		if (currentTestRun < 0)
			currentTestRun = 0;
		else if (currentTestRun > MaxTestRuns - 1)
			currentTestRun = MaxTestRuns - 1;
		ImGui::SameLine();
		ImGui::Text("Index: %d", currentTestRun);

		ImGui::InputText("##Loading", loadingFilename, MaxStringLength);
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			nctl::String filepath = nc::IFile::dataPath() + loadingFilename;
			if (nc::IFile::access(filepath.data(), nc::IFile::AccessMode::READABLE))
				loadTestRun(filepath.data(), currentTestRun);
			else
				LOGW_X("Cannot load file \"%s\" for index %u", filepath.data(), currentTestRun);
		}

		const TestRun &tr = testRuns[currentTestRun];
		const bool notLoaded = tr.filename.isEmpty();
		ImGui::Text("Filename: %s", notLoaded ? "N/A" : tr.filename.data());
		ImGui::Text("System: %s", notLoaded ? "N/A" : tr.system.data());
	}

	if (ImGui::CollapsingHeader("Test Run", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("System: %s", system());
		ImGui::Combo("Test", &currentTest, testNames, Tests::Count);
		int thousandIterations = numIterations / 1000;
		ImGui::SliderInt("Iterations", &thousandIterations, 1, MaxIterations / 1000, "%d K");
		numIterations = thousandIterations * 1000;
		ImGui::SliderInt("Repetitions", &numRepetitions, 1, MaxRepetitions);

		if (runningAllTests)
		{
			const float progress = (allTestsIndex + 1) / static_cast<float>(Tests::Count);
			progressMsg.format("%u / %u", allTestsIndex + 1, Tests::Count);
			ImGui::ProgressBar(progress, ImVec2(0.0f,0.0f), progressMsg.data());
			ImGui::NewLine();
		}
		else
		{
			if (ImGui::Button("Run"))
				runTest(t, numRepetitions, numIterations);
			ImGui::SameLine();
			if (ImGui::Button("Run All"))
				runningAllTests = true;

			ImGui::NewLine();
			ImGui::InputText("##Saving", savingFilename, MaxStringLength);
			ImGui::SameLine();
			if (ImGui::Button("Save"))
			{
				nctl::String filepath = nc::IFile::dataPath() + savingFilename;
				saveTestRun(filepath.data(), includeStatsWhenSaving);
			}
			ImGui::Checkbox("Include Statistics", &includeStatsWhenSaving);
		}
	}

	if (ImGui::CollapsingHeader("Results", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::ArrowButton("##LeftTest", ImGuiDir_Left))
			currentTest--;
		ImGui::SameLine();
		if (ImGui::ArrowButton("##RightTest", ImGuiDir_Right))
			currentTest++;
		if (currentTest < 0)
			currentTest = 0;
		else if (currentTest > Tests::Count - 1)
			currentTest = Tests::Count - 1;
		ImGui::SameLine();
		ImGui::Text("Test: %s", testNames[currentTest]);

		const TestRun &tr = testRuns[currentTestRun];
		const TestInfo &tri = tr.testInfos[currentTest];
		const bool canCompare = (tri.totalTime > 0.0f && t.totalTime > 0.0f);

		ImGui::Text("Iterations: %u", t.numIterations);
		if (tri.numIterations != t.numIterations)
		{
			ImGui::SameLine();

			if (t.numIterations != 0)
				ImGui::TextColored(Red, "(Comparing with %u iterations)", tri.numIterations);
			else
				ImGui::Text("(Test run loaded with %u iterations)", tri.numIterations);
		}

		ImGui::Text("Total Time: %f ms", t.totalTime);
		if (canCompare)
		{
			ImGui::SameLine();
			const ImVec4 color = (tri.totalTime > t.totalTime) ? Green : Red;
			ImGui::TextColored(color, "(%f ms, %.2fx)", tri.totalTime, t.totalTime / tri.totalTime);
		}
		ImGui::PlotHistogram("Times", t.times, t.numRepetitions, 0, nullptr, 0.0f, FLT_MAX, ImVec2(0.0f, 100.0f));
		ImGui::Text("Max Time: %f ms", t.maxTime);
		if (canCompare)
		{
			ImGui::SameLine();
			const ImVec4 color = (tri.maxTime > t.maxTime) ? Green : Red;
			ImGui::TextColored(color, "(%f ms, %.2fx)", tri.maxTime, t.maxTime / tri.maxTime);
		}
		ImGui::Text("Min Time: %f ms", t.minTime);
		if (canCompare)
		{
			ImGui::SameLine();
			const ImVec4 color = (tri.minTime > t.minTime) ? Green : Red;
			ImGui::TextColored(color, "(%f ms, %.2fx)", tri.minTime, t.minTime / tri.minTime);
		}
		ImGui::Text("Average: %f ms", t.average);
		if (canCompare)
		{
			ImGui::SameLine();
			const ImVec4 color = (tri.average > t.average) ? Green : Red;
			ImGui::TextColored(color, "(%f ms, %.2fx)", tri.average, t.average / tri.average);
		}
		ImGui::Text("Std. Deviation: %f ms", t.stdDeviation);
		if (canCompare)
		{
			ImGui::SameLine();
			ImGui::Text("(%f ms)", tri.stdDeviation);
		}
		ImGui::Text("Relative S.D.: %.2f %%", t.average > 0.0f ? 100.0f * t.stdDeviation / t.average : 0.0f);
		if (canCompare)
		{
			ImGui::SameLine();
			ImGui::Text("(%.2f %%)", tri.average > 0.0f ? 100.0f * tri.stdDeviation / tri.average : 0.0f);
		}
	}

	ImGui::End();
}

void MyEventHandler::onKeyReleased(const nc::KeyboardEvent &event)
{
	if (event.sym == nc::KeySym::ESCAPE || event.sym == nc::KeySym::Q)
		nc::theApplication().quit();
}
