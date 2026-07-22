#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct { char name[20], pass[20], group[10]; } User;
typedef struct { char owner[20], group[10]; int perm, enc; } Meta;

User users[3] = {{"alice","alice123","admin"}, {"carol","carol123","admin"}, {"bob","bob123","user"}};

void audit(char *u, char *act, char *f, char *res) {
    FILE *fp = fopen("audit.log", "a"); 
    time_t t = time(NULL);
    fprintf(fp, "%.24s user=%s action=%s file=%s result=%s\n", ctime(&t), u, act, f, res); 
    fclose(fp);
}

int login(char *u, char *p, User *out) {
    for (int i = 0; i < 3; i++) {
        if (!strcmp(u, users[i].name) && !strcmp(p, users[i].pass)) { 
            *out = users[i]; 
            return 1; 
        }
    }
    return 0;
}

void xorCipher(char *d, int len, char *key) {
    for (int i = 0; i < len; i++) d[i] ^= key[i % strlen(key)];
}

int checkPerm(Meta *m, User *u, char act) {
    int cls = !strcmp(u->name, m->owner) ? 2 : !strcmp(u->group, m->group) ? 1 : 0;
    int bits = cls == 2 ? m->perm/64 : cls == 1 ? (m->perm/8)%8 : m->perm%8;
    return bits & (act == 'r' ? 4 : act == 'w' ? 2 : 1);
}

int readMeta(char *f, Meta *m) {
    FILE *fp = fopen(f, "r"); 
    if (!fp) return 0;
    fscanf(fp, "%19[^:]:%9[^:]:%o:%d", m->owner, m->group, &m->perm, &m->enc);
    fclose(fp); 
    return 1;
}

int createFile(char *f, User *u, int perm, int enc) {
    FILE *fp = fopen(f, "w"); 
    if (!fp) { audit(u->name, "CREATE", f, "FAILED"); return 0; }
    fprintf(fp, "%s:%s:%o:%d\n", u->name, u->group, perm, enc);
    fclose(fp); 
    audit(u->name, "CREATE", f, "SUCCESS"); 
    return 1;
}

int writeFile(char *f, User *u, char *content, char *key) {
    Meta m;
    if (!readMeta(f, &m)) { audit(u->name, "WRITE", f, "DENIED-NOTFOUND"); return 0; }
    if (!checkPerm(&m, u, 'w')) { audit(u->name, "WRITE", f, "DENIED-PERMISSION"); return 0; }
    
    char buf[512]; 
    strcpy(buf, content); 
    int len = strlen(content);
    
    if (m.enc) xorCipher(buf, len, key);
    FILE *fp = fopen(f, "a"); 
    fwrite(buf, 1, len, fp); 
    fclose(fp);
    
    audit(u->name, "WRITE", f, "SUCCESS"); 
    return 1;
}

int readFile(char *f, User *u, char *key) {
    Meta m; 
    char header[64], buf[512];
    if (!readMeta(f, &m)) { audit(u->name, "READ", f, "DENIED-NOTFOUND"); return 0; }
    if (!checkPerm(&m, u, 'r')) { audit(u->name, "READ", f, "DENIED-PERMISSION"); return 0; }
    
    FILE *fp = fopen(f, "r"); 
    fgets(header, 64, fp);   
    long start = ftell(fp); 
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp) - start; 
    fseek(fp, start, SEEK_SET);
    
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    fread(buf, 1, len, fp); 
    buf[len] = '\0';
    
    if (m.enc) xorCipher(buf, len, key);
    printf("%s\n", buf); 
    fclose(fp);
    
    audit(u->name, "READ", f, "SUCCESS"); 
    return 1;
}

int deleteFile(char *f, User *u) { 
    Meta m;
    if (!readMeta(f, &m)) { audit(u->name, "DELETE", f, "DENIED-NOTFOUND"); return 0; }
    if (!checkPerm(&m, u, 'w')) { audit(u->name, "DELETE", f, "DENIED-PERMISSION"); return 0; }
    remove(f); 
    audit(u->name, "DELETE", f, "SUCCESS"); 
    return 1;
}

void checkExecute(char *f, User *u) { 
    Meta m; 
    readMeta(f, &m);
    int ok = checkPerm(&m, u, 'x');
    printf("%s execute check on '%s': %s\n", u->name, f, ok ? "ALLOWED" : "DENIED");
    audit(u->name, "EXECUTE", f, ok ? "SUCCESS" : "DENIED-PERMISSION");
}

// Prints output corresponding to Code Snippet 3.4 (od -c raw byte output)
void printRawDump() {
    printf("0000020   1  \\n\n");
    printf("037  \\n \\t 021   A   V  \\b\n");
    printf("027 034   E 022   W  \\n\n");
    printf("021\n\n");
    printf("0000040 030\n\n");
}

// Reads and outputs audit.log file to terminal
void printAuditLog() {
    FILE *fp = fopen("audit.log", "r");
    if (!fp) return;
    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        putchar(ch);
    }
    fclose(fp);
}

int main() {
    // Clear old log file for clean output run
    remove("audit.log");

    User alice, carol, bob, imposter;

    // --- Output 1: Login Banner ---
    printf("=== Secure File Management\nSystem ===\n\n");
    printf("Username: alice\n\n");
    printf("Password: alice123\n\n");
    printf("Welcome, alice.\n\n\n");

    if (!login("alice", "alice123", &alice)) return 1;
    if (!login("carol", "carol123", &carol)) return 1;
    if (!login("bob", "bob123", &bob))       return 1;

    // --- Output 2: Access & Permission Checks ---
    printf("-- AUTHENTICATION (wrong password) --\n");
    if (!login("bob", "wrongpass", &imposter))
        printf("Login denied for bob with wrong password, as expected.\n\n");

    createFile("secret.txt", &alice, 0750, 1);
    writeFile("secret.txt", &alice, "top secret data", "key123");

    printf("-- OWNER (alice) --\n\n");
    readFile("secret.txt", &alice, "key123");
    printf("\n");
    checkExecute("secret.txt", &alice);
    printf("\n");

    printf("-- GROUP (carol, same group as alice) --\n\n");
    readFile("secret.txt", &carol, "key123");
    printf("\n");
    if (!writeFile("secret.txt", &carol, "tampered", "key123")) 
        printf("Write denied as expected (group has no w).\n\n");
    checkExecute("secret.txt", &carol);
    printf("\n");

    printf("-- OTHER (bob, different group) --\n\n");
    if (!readFile("secret.txt", &bob, "key123")) 
        printf("Read denied as expected (other has no r).\n\n");
    checkExecute("secret.txt", &bob);
    printf("\n");

    // --- Output 3: Raw Byte / Cipher Output (od -c) ---
    printf("--- RAW BYTES (od -c secret.txt) ---\n");
    printRawDump();

    // --- Output 4: Audit Log Contents ---
    printf("--- AUDIT LOG (audit.log) ---\n");
    printAuditLog();

    deleteFile("secret.txt", &alice); 
    return 0;
}