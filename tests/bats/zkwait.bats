#!/usr/bin/env bats

load test_helper

@test "zkwait help" {
	run src/zkwait -h
	[ "$status" = 0 ]
	[ "${lines[1]}" = "Usage: src/zkwait [OPTION] NODE" ]
}

@test "zkwait failes without a server" {
	run src/zkwait /node
	[ "$status" = 1 ]
}

@test "zkwait failes without a node" {
  run src/zkwait --server 'yolo'
  [ "$status" = 1 ]
} 

@test "zkwait failes with a unreachable server" {
  run src/zkwait --server '127.0.0.1:1' $(mktemp --dry-run /tmp.XXXX)
  [ "$status" = 1 ]
}

@test "zkwait returns if node does not exists" {
  NODE="$(mktemp --dry-run /tmp.XXXX)"
  run src/zkwait --server 'zoo1:2181' $NODE 
  [ "$status" = 0 ]
}

@test "zkwait does not return if node exists" {
  NODE="$(mktemp --dry-run /tmp.XXXX)"
  run src/zktouch --server 'zoo1:2181' $NODE
  [ "$status" = 0 ]
  
  run timeout 5s src/zkwait --server 'zoo1:2181' $NODE
  [ "$status" = 124 ]
}

@test "zkwait returns if node is removed" {
  NODE="$(mktemp --dry-run /tmp.XXXX)"

  run src/zktouch --server 'zoo1:2181' $NODE
  [ "$status" = 0 ]

  (sleep 5; src/zkrm --server 'zoo1:2181' $NODE)&

  run timeout 20 src/zkwait --server 'zoo1:2181' $NODE
  [ "$status" = 0 ]
}
