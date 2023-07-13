
cpp_dockerfile = Dockerfile
cpp_docker_image = strash_api_client_dev
cpp_docker_image_ver = 0.0.0
cpp_docker_container= StrashApiCli_tests

debug = false

strash_test = false

main: clean build run_tests


build:
	mkdir -p build
	sh -c " \
		cd build; \
		cmake .. `${strash_test} &&  echo '-DTEST_REALSTRASH=ON'` `${debug} && echo '-DCMAKE_BUILD_TYPE=Debug'`; \
		make -j4"

run_tests:
	sh -c " \
		cd build/tests; \
		./StrashApiClient_tests"

debug_tests:
	sh -c " \
		cd build/tests; \
		gdb ./StrashApiClient_tests"

clean:
	rm -rf build
	rm -rf lib





.docker_start:
	systemctl is-active docker.service || systemctl start docker.service

cpp_docker_build: .docker_start
	docker build \
		--build-arg CPP_DEBUG=$(debug)\
		--build-arg STRASH=${strash_test}\
		--rm -t $(cpp_docker_image):$(cpp_docker_image_ver) \
		-f $(cpp_dockerfile) \
		.

cpp_docker_run:
	docker run \
		--rm --name $(cpp_docker_container) \
		-t $(cpp_docker_image):$(cpp_docker_image_ver)

cpp_docker_bash: cpp_docker_build
	docker run \
		--rm --name $(cpp_docker_container) -it \
		$(cpp_docker_image):$(cpp_docker_image_ver) \
		bash

cpp_docker: cpp_docker_build cpp_docker_run

cpp_docker_prune_all:
	docker image prune
	docker container prune
	docker volume prune



cpp_compose_build: .docker_start
	docker-compose -f tests/compose/docker-compose.yml build --build-arg CPP_DEBUG=$(debug) --build-arg STRASH=$(strash_test)

cpp_compose_run: cpp_compose_build
	docker-compose -f tests/compose/docker-compose.yml up

cpp_compose: cpp_compose_build cpp_compose_run

cpp_compose_exec_debug:
	docker exec -it strashapi-cpp-client bash -c "cd /strash_api_client/build/tests; gdb ./StrashApiClient_tests"
