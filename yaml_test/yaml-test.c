#include <yaml.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void insertIntfaces(yaml_document_t *document, yaml_node_t *node)
{
    int properties, key, value, root = node - document->nodes.start + 1;

    /* interface ath00 */
    properties = yaml_document_add_mapping(document, NULL, YAML_BLOCK_MAPPING_STYLE);
    yaml_document_append_sequence_item(document, root, properties);
    key = yaml_document_add_scalar(document, NULL, "interfacexxx", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "ath00", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "threads", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "auto", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "cluster-id", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "99", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "defrag", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "yes", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "cluster-type", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "cluster_flow", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "copy-mode", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "ips", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "copy-iface", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "tapSec_ath00", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "buffer-size", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "32768", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);

    /* interface tapSec_ath00 */
    properties = yaml_document_add_mapping(document, NULL, YAML_BLOCK_MAPPING_STYLE);
    yaml_document_append_sequence_item(document, root, properties);
    key = yaml_document_add_scalar(document, NULL, "interface", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "tapSec_ath00", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "threads", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "auto", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "cluster-id", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "98", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "defrag", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "yes", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "cluster-type", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "cluster_flow", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "copy-mode", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "ips", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "copy-iface", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "ath00", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);
    key = yaml_document_add_scalar(document, NULL, "buffer-size", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(document, NULL, "32768", -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(document, properties, key, value);

    /* more interfaces */
}

int main(int argc, char *argv[])
{
    FILE *file;
    yaml_parser_t parser;
    yaml_emitter_t emitter;
    yaml_document_t document;
    int done = 0;

    if (argc > 1)
        file = fopen(argv[1], "rb");
    else
        file = fopen("suricata.yaml", "rb");
    assert(file);
    assert(yaml_parser_initialize(&parser));
    yaml_parser_set_input_file(&parser, file);
    yaml_emitter_initialize(&emitter);
    yaml_emitter_set_output_file(&emitter, stdout);
    yaml_emitter_open(&emitter);
    while (!done)
    {
        if (!yaml_parser_load(&parser, &document)) {
            printf("yaml_parser_load() return error\n");
            break;
        }

        done = (!yaml_document_get_root_node(&document));
        if (!done) {
            yaml_node_t *node;
            for (node = document.nodes.start; node != document.nodes.top; node++) {
                if (node->type == YAML_SCALAR_NODE && !strcmp(node->data.scalar.value, "af-packet")) {
#if 0
                    printf("node %s ID = %d\n", node->data.scalar.value, (int)(node - document.nodes.start));
                   	free(node->data.scalar.value);
                   	node->data.scalar.value = malloc(100);
                   	snprintf(node->data.scalar.value, "%s", "XXXXXXXXX");
                   	node->data.scalar.length = strlen(node->data.scalar.value);
#endif
                    insertIntfaces(&document, node+1);
                    break;
                }
            }
            yaml_emitter_dump(&emitter, &document);
            yaml_emitter_flush(&emitter);
        }
        else {
            yaml_document_delete(&document);
        }
    }

    yaml_parser_delete(&parser);
    fclose(file);
    yaml_emitter_close(&emitter);
    yaml_emitter_delete(&emitter);

    return 0;
}
