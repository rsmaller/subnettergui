import os

os.system("cmake . -G \"Visual Studio 17 2022\"")
os.system("cmake --build . --config Release")