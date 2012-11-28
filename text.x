/* Write code here */
typedef string textout<2000>;

/* A node in the directory listing */

struct pingrequest {
   opaque clientId<>;
  };

struct pingresponse {
   int output;
  };

struct statrequest {
    opaque clientId<>;
};

struct statresponse{
    int pingnum;
    int swapnum;
    int hashnum;
    int checknum;
    int statnum;
};

struct swaprequest {
    opaque clientId<>;
    int filesize;
    opaque textin<>;
    int isFinished;
    int part;
    int swapno;
};

struct swapresponse {
    opaque out<>;
    int swapno;
};

struct hashrequest {
    opaque clientId<>;
    int isFinished;
    opaque textin<>;
    int part;
    int hash;
    int hasParts;
    int filesize;
};

struct hashresponse {
    int hash;
};

struct checkrequest {
    opaque clientId<>;
    int isFinished;
    opaque textin<>;
    int servHash;
    int part;
    int hasParts;
    int clieHash;
    int filesize;
};

struct checkresponse {
    int hashAcum;
    opaque isCorrect[1];
};





program SERVER {
   version SERVERPROGVERS {
     pingresponse PINGPROC(pingrequest) = 1;
     statresponse STATPROC(statrequest) = 2;
     swapresponse SWAPPROC(swaprequest) = 3;
     hashresponse HASHPROC(hashrequest) = 4;
     checkresponse CHECKPROC(checkrequest) = 5;
   } = 1;
} = 777777777;
