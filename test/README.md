# Testing

1. Install [Docker](https://docs.docker.com/engine/install/)
2. Launch a terminal and change to the project root
3. Type `docker compose run test` which will build the Docker image, instantiate a container, compile the code, and run all the tests

During the image build process a few dependecies are fetched and installed via the apt package manager *inside the container* while the host system will remain untouched.

The sys-sage repository is mounted into the container, i.e. changes made to files are immediately visible to the container.

Otherwise useful Docker commands:
- `docker compose build`: Just rebuild the Docker image, i.e. reinstalls dependencies.
- `docker compose build --no-cache`: Enforce a clean rebuild of the Docker image.
- `docker compose run test "cat /proc/cpuinfo"`: Run a command inside the container.
