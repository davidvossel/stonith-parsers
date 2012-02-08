/* 
 * Copyright (C) 2012
 * David Vossel  <dvossel@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <crm/crm.h>
#include "standalone_config.h"

struct device {
	char *name;
	char *agent;
	char *hostlist;
	char *hostmap;

	struct {
		char *key;
		char *val;
	} key_vals[STANDALONE_CFG_MAX_KEYVALS];
	int key_vals_count;

	struct device *next;
};

struct topology {
	char *node_name;
	struct {
		char *device_name;
		unsigned int level;
	} priority_levels[STANDALONE_CFG_MAX_KEYVALS];
	int priority_levels_count;

	struct topology *next;
};

struct device *device_list;
struct topology *topology_list;

static struct device *find_device(const char *name)
{
	struct device *dev = NULL;

	for (dev = device_list; dev != NULL; dev = dev->next) {
		if (!strcasecmp(dev->name, name)) {
			break;
		}
	}

	return dev;
}

static struct topology *find_topology(const char *name)
{
	struct topology *topo = NULL;

	for (topo = topology_list; topo != NULL; topo = topo->next) {
		if (!strcasecmp(topo->node_name, name)) {
			break;
		}
	}

	return topo;
}

static void add_device(struct device *dev)
{
	dev->next = device_list;
	device_list = dev;
}

static void add_topology(struct topology *topo)
{
	topo->next = topology_list;
	topology_list = topo;
}

int standalone_cfg_add_device(const char *device, const char *agent)
{
	struct device *dev;

	/* just ignore duplicates */
	if (find_device(device)) {
		return 0;
	}
//	crm_malloc0(dev, sizeof(*dev));

	if (!(dev = calloc(1, sizeof(*dev)))) {
		return -1;
	}

	dev->name = strdup(device);
	dev->agent = strdup(agent);
	add_device(dev);

	return 0;
}

int standalone_cfg_add_device_options(const char *device, const char *key, const char *value)
{
	struct device *dev;

	if (!(dev = find_device(device))) {
		return -1;
	}

	dev->key_vals[dev->key_vals_count].key = strdup(key);
	dev->key_vals[dev->key_vals_count].val = strdup(value);
	dev->key_vals_count++;

	return 0;
}

int standalone_cfg_add_node(const char *node, const char *device, const char *ports)
{
	// TODO
	return 0;
}

int standalone_cfg_add_node_priority(const char *node, const char *device, unsigned int level)
{
	struct topology *topo;
	int new = 1;
	int i;

	if ((topo = find_topology(node))) {
		new = 0;
//	crm_malloc0(topo, sizeof(*topo));
	} else if ((topo = calloc(1, sizeof(*topo)))) {
		topo->node_name = strdup(node);
	} else {
		return -1;
	}

	topo->priority_levels[topo->priority_levels_count].device_name = strdup(device);
	topo->priority_levels[topo->priority_levels_count].level = level;
	topo->priority_levels_count++;

	if (new) {
		add_topology(topo);
	}

	return 0;
}

static int destroy_devices(struct device *dev)
{

	return 0;
}

int standalone_cfg_commit(void)
{
	struct device *dev = NULL;
	struct topology *topo = NULL;
	int i;

	/* TODO - for now just printing out, but build xml once intergrated with stonith. */
	printf("commit!\n");
	printf("--- Devices\n");

	for (dev = device_list; dev != NULL; dev = dev->next) {
		printf("    name: %s\n", dev->name);
		printf("    agent: %s\n", dev->agent);
		printf("    hostlist: %s\n", dev->hostlist);
		printf("    hostmap: %s\n", dev->hostmap);
		for (i = 0; i < dev->key_vals_count; i++) {
			printf("        %s=%s\n", dev->key_vals[i].key, dev->key_vals[i].val);
		}
		printf("\n");
	}

	printf("--- Topology\n");

	for (topo = topology_list; topo != NULL; topo = topo->next) {
		printf("    node: %s\n", topo->node_name);
		for (i = 0; i < topo->priority_levels_count; i++) {
			printf("        %d=%s\n",
				topo->priority_levels[i].level,
				topo->priority_levels[i].device_name);
		}
		printf("\n");
	}

	return 0;
}
