#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <byteswap.h> // For be32toh


#define MAX_LOCATIONS 100
#define OPCODE_SIZE 27
#define CONDITION_SIZE 14

// Define the structure for instruction
typedef struct {
    const char* mnemonic;
    uint32_t binary_representation;

} instruction_t;

typedef struct{
  char s_location[20];
  uint32_t location;
} location_t;

typedef struct{
  const char* s_bcondition;
  uint32_t bcondition;
}bcondition_t;

typedef union {
    int i;
    float f;
} intfloat;

instruction_t instruction[] = {
  { "ADD",    0b10001011000 }, //R
  { "ADDI",   0b1001000100  }, //I
  { "AND",    0b10001010000 }, //R
  { "ANDI",   0b1001001000  }, //I
  { "B",      0b000101      }, //B
  { "BL",     0b100101      }, //B
  { "BR",     0b11010110000 }, //R
  { "B.cond", 0b01010100    }, //CB
  { "CBNZ",   0b10110101    }, //CB
  { "CBZ",    0b10110100    }, //CB
  { "DUMP",   0b11111111110 }, //R
  { "EOR",    0b11001010000 }, //R
  { "EORI",   0b1101001000  }, //I
  { "HALT",   0b11111111111 }, //R
  { "LDUR",   0b11111000010 }, //D
  { "LSL",    0b11010011011 }, //R
  { "LSR",    0b11010011010 }, //R
  { "MUL",    0b10011011000 }, //R
  { "ORR",    0b10101010000 }, //R
  { "ORRI",   0b1011001000  }, //I
  { "PRNL",   0b11111111100 }, //R
  { "PRNT",   0b11111111101 }, //R
  { "STUR",   0b11111000000 }, //D
  { "SUB",    0b11001011000 }, //R
  { "SUBI",   0b1101000100  }, //I
  { "SUBIS",  0b1111000100  }, //I
  { "SUBS",   0b11101011000 } //R
};

bcondition_t branch_conditions[] = {
  {"EQ", 0x0},
  {"NE", 0x1},
  {"HS", 0x2},
  {"LO", 0x3},
  {"MI", 0x4},
  {"PL", 0x5},
  {"VS", 0x6},
  {"VC", 0x7},
  {"HI", 0x8},
  {"LS", 0x9},
  {"GE", 0xA},
  {"LT", 0xB},
  {"GT", 0xC},
  {"LE", 0xD}
};

void float_bits(intfloat i)
{
  int j;

  for (j = 31; j >= 0; j--) {
    printf("%d", (i.i >> j) & 0x1);
  }
  printf("\n");
}

/*   abcdefghijkl  <- these letters are variables for bits
 * & 001101001010
 * --------------
 *   00cd0f00i0k0
 */

void print_ieee754_fields(intfloat i)
{
  // Printing mantissa prints a leading 0 bit as an artifact of the
  // hex output.  That highest bit is not actually part of the mantissa.
  printf("f: %f\ts: %d\tbe: %d\tue: %d\tm: %#x\n", i.f,
         (i.i >> 31) & 0x1, (i.i >> 23) & 0xff, ((i.i >> 23) & 0xff) - 127,
         i.i & 0x7fffff);
}

char* findOrAddLocation(location_t locations[], int *numLocations, int location) {

    for (int i = 0; i < *numLocations; i++) {
        if (locations[i].location == location) {
            return locations[i].s_location;
        }
    }

    // If location not found, add it to array
    sprintf(locations[*numLocations].s_location, "label_%d", *numLocations);
    locations[*numLocations].location = location;
    (*numLocations)++;

  

    // Return the newly assigned label
    return locations[*numLocations-1].s_location;
}



// Define your decode function , uint32_t *bprogram
//void decode(uint32_t b_instruction, uint32_t *bprogram) {
void decode(uint32_t b_instruction, uint32_t *bprogram) {

  char s_assembly[100] = "";

  int num_label = 0;

  location_t locations[MAX_LOCATIONS];


  //Extract opcode 
  uint32_t R_D_opcode = (b_instruction >> 21) & 0x7FF; //11 bits

  uint32_t I_opcode = (b_instruction >> 22) & 0x3FF;  //10 bits

  uint32_t B_opcode = (b_instruction >> 26) & 0x3F;  //6 bits

  uint32_t CB_opcode = (b_instruction >> 24) &0xFF; //8 bits

//Iterate over the instruction array
for(int i = 0; i < OPCODE_SIZE; i++){

  //R and D format
  if(R_D_opcode == instruction[i].binary_representation){

  //R format
  if( !(R_D_opcode == 0b11111000010) && !(R_D_opcode == 0b11111000000)){

   int Rd = b_instruction & 0x1F; // 5 bits Rd
   char s_Rd[5];

   sprintf(s_Rd, "X%d", Rd);

   if(Rd == 28){
    sprintf(s_Rd, "SP");
   }
 
    else if(Rd == 29){
    sprintf(s_Rd, "FP");
    }

   else if(Rd == 30){
    sprintf(s_Rd, "LR");
   }

   else if(Rd == 31 ){
    sprintf(s_Rd, "XZR");
   }

   int Rn = (b_instruction >> 5) & 0x1F; //5 bits Rn
   char s_Rn[5];

   sprintf(s_Rn, "X%d", Rn);

    if(Rn == 28){
    sprintf(s_Rn, "SP");
   }
 
    else if(Rn == 29){
    sprintf(s_Rn, "FP");
    }

   else if(Rn == 30){
   sprintf(s_Rn, "LR");
   }

   else if(Rn == 31 ){
   sprintf(s_Rn, "XZR");
   }

   int Rm = (b_instruction >> 16) & 0x1F; //5 bits Rm
   char s_Rm[5];

   sprintf(s_Rm, "X%d", Rm);

      if(Rm == 28){
      sprintf(s_Rm, "SP");
   }
 
    else if(Rm == 29){
     sprintf(s_Rm, "FP");
    }

   else if(Rm == 30){
    sprintf(s_Rm, "LR");
   }

   else if(Rm == 31 ){
    sprintf(s_Rm, "XZR");
   }

   int shamt = (b_instruction >> 10) & 0x3F;

   switch(R_D_opcode){

    case 0b10001011000:
    sprintf(s_assembly, "ADD %s, %s, %s", s_Rd, s_Rn, s_Rm);
    break;

    case 0b10001010000:
    sprintf(s_assembly, "AND %s, %s, %s", s_Rd, s_Rn, s_Rm);
    break;

    case 0b11010110000:
    sprintf(s_assembly, "BR %s", s_Rn);
    break;

    case 0b11111111110:
    sprintf(s_assembly, "DUMP ");
    break;

    case 0b11001010000:
    sprintf(s_assembly, "EOR %s, %s, %s", s_Rd, s_Rn, s_Rm);
    break;

    case 0b11111111111:
    sprintf(s_assembly, "HALT ");
    break;

    case 0b10101010000:
    sprintf(s_assembly, "ORR %s, %s, %s", s_Rd, s_Rn, s_Rm);
    break;

    case 0b11111111100:
    sprintf(s_assembly, "PRNL ");
    break;

    case 0b11111111101:
    sprintf(s_assembly, "PRNT %s", s_Rd);
    break;

    case 0b10011011000:
    sprintf(s_assembly, "MUL %s, %s, %s", s_Rd, s_Rn, s_Rm);
    break;

    case 0b11001011000:
    sprintf(s_assembly, "SUB %s, %s, %s", s_Rd, s_Rn, s_Rm);
    break;

    case 0b11101011000:
    sprintf(s_assembly, "SUBS %s, %s, %s", s_Rd, s_Rn, s_Rm);
    break;

    case 0b11010011011:
    sprintf(s_assembly, "LSL %s, %s, %d", s_Rd, s_Rn, shamt);
    break;

    case 0b11010011010:
    sprintf(s_assembly, "LSR %s, %s, %d", s_Rd, s_Rn, shamt);
    break;

    default:
    break;

   }

    }

//D format
 else {

   int Rt = b_instruction & 0x1F; // 5 bits Rt
   char s_Rt[5];

   sprintf(s_Rt, "X%d", Rt);

   if(Rt == 28){
    sprintf(s_Rt, "SP");
   }
 
    else if(Rt == 29){
    sprintf(s_Rt, "FP");
    }

   else if(Rt == 30){
    sprintf(s_Rt, "LR");
   }

   else if(Rt == 31 ){
    sprintf(s_Rt, "XZR");
   }

   int Rn = (b_instruction >> 5) & 0x1F; //5 bits Rn
   char s_Rn[5];

   sprintf(s_Rn, "X%d", Rn);

    if(Rn == 28){
    sprintf(s_Rn, "SP");
   }
 
    else if(Rn == 29){
    sprintf(s_Rn, "FP");
    }

   else if(Rn == 30){
   sprintf(s_Rn, "LR");
   }

   else if(Rn == 31 ){
   sprintf(s_Rn, "XZR");
   }

   int address = (b_instruction >> 12) & 0x1FF; //9 bits address

   switch (R_D_opcode)
   {

   case 0b11111000010:
    sprintf(s_assembly, "LDUR %s, [%s, #%d]", s_Rt, s_Rn, address);
    break;

   case 0b11111000000:
    sprintf(s_assembly, "STUR %s, [%s, #%d]", s_Rt, s_Rn, address);
    break;
   
   default:
    break;
     }

  }

 bprogram[i] = b_instruction;
  printf("%s \r\n", s_assembly);
  return;

  }
  
  //I format
  else if(I_opcode == instruction[i].binary_representation){

   int Rd = b_instruction & 0x1F; // 5 bits Rd
   char s_Rd[5];

   sprintf(s_Rd, "X%d", Rd);

   if(Rd == 28){
    sprintf(s_Rd, "SP");
   }
 
    else if(Rd == 29){
    sprintf(s_Rd, "FP");
    }

   else if(Rd == 30){
    sprintf(s_Rd, "LR");
   }

   else if(Rd == 31 ){
    sprintf(s_Rd, "XZR");
   }

   int Rn = (b_instruction >> 5) & 0x1F; //5 bits Rn
   char s_Rn[5];

   sprintf(s_Rn, "X%d", Rn);

    if(Rn == 28){
    sprintf(s_Rn, "SP");
   }
 
    else if(Rn == 29){
    sprintf(s_Rn, "FP");
    }

   else if(Rn == 30){
   sprintf(s_Rn, "LR");
   }

   else if(Rn == 31){
   sprintf(s_Rn, "XZR");
   }

   int immediate = (b_instruction >> 10) & 0xFFF;

   if(immediate >= 2048){ //Check if the sign bit == 1

   immediate -= 4096; //Make it a negative number 

   }


   switch (I_opcode)
   {
    case 0b1001000100:
    sprintf(s_assembly, "ADDI %s, %s, #%d", s_Rd, s_Rn, immediate);
    break;

    case 0b1001001000:
    sprintf(s_assembly, "ANDI %s, %s, #%d", s_Rd, s_Rn, immediate);
    break;

    case 0b1101001000:
    sprintf(s_assembly, "EORI %s, %s, #%d", s_Rd, s_Rn, immediate);
    break;

    case 0b1011001000:
    sprintf(s_assembly, "ORRI %s, %s, #%d", s_Rd, s_Rn, immediate);
    break;

    case 0b1101000100:
    sprintf(s_assembly, "SUBI %s, %s, #%d", s_Rd, s_Rn, immediate);
    break;

    case 0b1111000100:
    sprintf(s_assembly, "SUBIS %s, %s, #%d", s_Rd, s_Rn, immediate);
    break;
   
    default:
    break;
   }

   bprogram[i] = b_instruction;
    printf("%s \r\n", s_assembly);
    return; 

  }

  else if(B_opcode == instruction[i].binary_representation){

   int location = b_instruction & 0x3FFFFFF;
   char* label;

   label = findOrAddLocation(locations, &num_label, location);

   if(B_opcode == 0b000101){

    sprintf(s_assembly, "B %s", label);
   }

   else if(B_opcode == 0b100101){
    sprintf(s_assembly, "BL %s", label);
   }

   bprogram[i] = b_instruction;
    printf("%s \r\n", s_assembly);
    return; 

  }

  else if(CB_opcode == instruction[i].binary_representation){


  if(CB_opcode == 0b10110101 || CB_opcode == 0b10110100){

   int reg = b_instruction & 0x1F; // 5 bits reg
   char s_reg[5];

   sprintf(s_reg, "X%d ", reg);
   
   char instr[5];

   if(CB_opcode == 0b10110101){

    sprintf(instr, "CBNZ");
   }

   else if(CB_opcode == 0b10110100){

     sprintf(instr, "CBZ");

   }

   int location = (b_instruction >> 5) & 0x7FFFF;
   char* label;

   label = findOrAddLocation(locations, &num_label, location);

   bprogram[i] = b_instruction;
   sprintf(s_assembly, "%s %s, %s", instr, s_reg, label);
   

    }
 
    else if(CB_opcode == 0b01010100){ //B.condition 

   int location = (b_instruction >> 5) & 0x7FFFF;
   char* label;

   label = findOrAddLocation(locations, &num_label, location);

   int cond = b_instruction & 0x1F; //check for conditions

    switch(cond){

      case 0x0:
      sprintf(s_assembly, "B.EQ %s", label);
      break;
      
      case 0x1:
      sprintf(s_assembly, "B.NE %s", label);
      break;
 
      case 0x2:
      sprintf(s_assembly, "B.HS %s", label);
      break;

      case 0x3:
      sprintf(s_assembly, "B.LO %s", label);
      break;

      case 0x4:
      sprintf(s_assembly, "B.MI %s", label);
      break;

      case 0x5:
      sprintf(s_assembly, "B.PL %s", label);
      break;

      case 0x6:
      sprintf(s_assembly, "B.VS %s", label);
      break;

      case 0x7:
      sprintf(s_assembly, "B.VC %s", label);
      break;

      case 0x8:
      sprintf(s_assembly, "B.HI %s", label);
      break;

      case 0x9:
      sprintf(s_assembly, "B.LS %s", label);
      break;

      case 0xA:
      sprintf(s_assembly, "B.GE %s", label);
      break;

      case 0xB:
      sprintf(s_assembly, "B.LT %s", label);
      break;

      case 0xC:
      sprintf(s_assembly, "B.GT %s", label);
      break;

      case 0xD:
      sprintf(s_assembly, "B.LE %s", label);
      break;

      default:
      break;

    }

    }

   bprogram[i] = b_instruction;
   printf("%s \r\n", s_assembly);
   return;

  }

  }
}

int main(int argc, char *argv[]) {
    int binary = 1; // Enable binary mode
    int fd;
    struct stat buf;
    uint32_t *program, *bprogram;
    size_t i;
    // Your other variables and code...

    // Check if binary mode is enabled
    if (binary) {
        // Open binary file
        fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            return 1;
        }

        // Fetch file information
        if (fstat(fd, &buf) == -1) {
            perror("Error getting file information");
            close(fd);
            return 1;
        }

        // Memory map the file
        program = mmap(NULL, buf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        if (program == MAP_FAILED) {
            perror("Error mapping file to memory");
            close(fd);
            return 1;
        }

        // Allocate memory for bprogram
        bprogram = calloc(buf.st_size / sizeof(uint32_t), sizeof(uint32_t));
        if (bprogram == NULL) {
            perror("Error allocating memory");
            munmap(program, buf.st_size);
            close(fd);
            return 1;
        }

        // Convert instructions and decode
        for (i = 0; i < (buf.st_size / sizeof(uint32_t)); i++) {
            program[i] = be32toh(program[i]);
            decode(program[i], bprogram + i); // Pass the address of the current index in bprogram
        }

        // Cleanup
        free(bprogram);
        munmap(program, buf.st_size);
        close(fd);
    }

    return 0;
}


/*int main(int argc, char *argv[]) {
    int binary = 1; // Enable binary mode
    int fd;
    struct stat buf;
    uint32_t *program, *bprogram;
    size_t i;
  

    // Check if binary mode is enabled
    if (binary) {
        // Open binary file
        fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            return 1;
        }

        // Fetch file information
        if (fstat(fd, &buf) == -1) {
            perror("Error getting file information");
            close(fd);
            return 1;
        }

        // Memory map the file
        program = mmap(NULL, buf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        if (program == MAP_FAILED) {
            perror("Error mapping file to memory");
            close(fd);
            return 1;
        }

        // Allocate memory for bprogram
        bprogram = calloc(buf.st_size / sizeof(uint32_t), sizeof(uint32_t));
        if (bprogram == NULL) {
            perror("Error allocating memory");
            munmap(program, buf.st_size);
            close(fd);
            return 1;
        }

        // Convert instructions and decode
        for (i = 0; i < (buf.st_size / sizeof(uint32_t)); i++) {
           program[i] = be32toh(program[i]);
           decode(program[i]);

        }

        // Cleanup
        free(bprogram);
        munmap(program, buf.st_size);
        close(fd);
    }

    return 0;
}

int main(int argc, char *argv[]){

   uint32_t binary_instruction = 0b11111000010000001000000110001001;

    decode(binary_instruction);

    return 0;

}*/