#pragma once

static int resolve_ipv4_address(const char *hostname, char *ipv4, size_t ipv4_len);
void *requester(void *arg);
void *resolver(void *arg);