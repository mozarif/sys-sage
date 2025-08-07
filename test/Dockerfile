FROM ubuntu:25.04

RUN apt-get update

# Install build tools
RUN apt-get install -yy g++ cmake ninja-build git

# Install dependencies
RUN apt-get install -yy libxml2-dev nlohmann-json3-dev python3 pybind11-dev
