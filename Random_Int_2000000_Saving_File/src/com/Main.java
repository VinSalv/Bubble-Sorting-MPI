/* 
* Course: High Performance Computing 2021/2022
* 
* Lecturer: Francesco Moscato   fmoscato@unisa.it
*
* Group:
* Lamberti      Martina     0622701476  m.lamberti61@studenti.unisa.it
* Salvati       Vincenzo    0622701550  v.salvati10@studenti.unisa.it
* Silvitelli    Daniele     0622701504  d.silvitelli@studenti.unisa.it
* Sorrentino    Alex        0622701480  a.sorrentino120@studenti.unisa.it
*
* Copyright (C) 2021 - All Rights Reserved
*
* This file is part of EsameHPC.
*
* Contest-OMP is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Contest-OMP is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Contest-OMP.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
    @file Main.java
*/

// PURPOSE OF THE FILE: The function generates random numbers and save them into a file. 

package com;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Random;

public class Main {
    public static void main(String[] args) {
        // Create file
        File f = new File("random_number.txt");
        if (f.exists()) { // Check if the file exist
            System.out.println("random_number exist !");
            return;
        }
        try {
            // Create flow of character
            FileWriter fw = new FileWriter(f);
            // Define how to manage the flow
            PrintWriter pw = new PrintWriter(fw);
            // Generate 2000000 random int to save in "random_number.txt"
            int min = 0;
            int max = 999999;
            Random random = new Random();
            for (int i = 0; i < 2050000; i++)
                pw.printf("%d\n", random.nextInt(max - min + 1) + min);
        } catch (IOException e) {
            e.printStackTrace();
        }
        System.out.println("End");
    }
}
