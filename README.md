# Trabajo-Practico-Sistemas-Operativos  
  
# Ejercicio 1  
  
El objetivo es ver el comportamiento de los procesos.   
  
-> COMO EJECUTAR EL ARCHIVO:  
  
  1)Abra la terminal en la ubicacion del archivo Ej1.c  
  2)Escriba en la terminal 'gcc -c Ej1.c'  
  3)Ahora escriba en la terminal 'gcc Ej1.o -o cocina'
  4)Ya puede ejecutar el archivo usando './cocina'  
  
-> FUNCIONAMIENTO  
  
El programa dara la opcion de "cocinar" tres platos. Cada plato se cocina mediante el orden:   
  Cortar (Divido)->Cocinar(Sumo)->Picar(Multiplico)->Empaltar(Ordeno de menor a mayor)    
  
Durante cada operacion se le dara un intervalo detiempo entre 10 y 20 segundos, junto al PID del proceso para que pueda consultar tranquilamente en la terminal.   
  
->LOTE DE PRUEBAS  
  
Cada plato representa una operacion matematica con un mismo numero para todas las operaciones siendo:  
Patel de papa: Divido, sumo y multiplico por 2  
Guiso de Lentejas: Divido, sumo y multiplico por 5  
Locro: Divido, sumo y multiplico por 8  

El plato base con el que empezaran las operaciones sera el mismo lote de pruebas siendo: {30, 10, 60, 20, 8}. Los resultados esperados son:  
Pastel de Papa: {12,14,24,34,64}  
Guiso de lentejas:{30,35,45,55,85}  
Locro:{72, 72, 80, 88, 120}  
  
*Nota: Al dividir por numero impar se redondea para abajo.*  

# Ejercicio 2

-------------------------------------------------
->INSTRUCCIONES

1)Abrir la terminal en la carpeta. 
2) Hacer un MakeFile (en bash/ubuntu), para esto escribir 'nano Makefile' y copiar:
'
CC = gcc
CFLAGS = -Wall -pthread

all: Servidor Cliente

Servidor: Servidor.c
        $(CC) $(CFLAGS) Servidor.c -o Servidor

Cliente: Cliente.c
        $(CC) $(CFLAGS) Cliente.c -o Cliente

clean:
        rm -f Servidor Cliente
'

Para terminar:
1- Ctrl + O para guardar
2- Enter para confirmar
3- Ctrl + X para salir

3) Compilar archivo makefile con
Primero:   make clean
Luego: 	   make

->FUNCIONAMIENTO

Para probarlo, abrir una consola (en el directorio del proyecto) y escribir

'./Servidor'

Ahora en otra consola (abierta en el directorio del proyecto) y escribir

'./Cliente'

------------------------------------------------
Las acciones disponibles para la consola abierta con la aplicacion cliente son las siguientes:

CORTAR:palabra
COCINAR:palabra
EMPLATAR:palabra

SUMA:Num1:Num2
RESTA:Num1:Num2

INVERSO:palabra
MAYUS:palabra

-----------------------------------------------------------
Para cerrar el Cliente presionar control+c. El servidor se cerrara automaticamente al no haber clientes.
