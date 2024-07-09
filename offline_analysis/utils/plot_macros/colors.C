void colors(int ci1=0, int ci2 = 0) {

   TObjArray *colors = (TObjArray*) gROOT->GetListOfColors();
   Int_t ncolors = colors->GetSize();
   if (ci2==0) ci2 = ncolors;
   TColor *color = 0;
   Int_t nc =0 ;

   printf("   +------+-------+-------+-------+-------+--------------------+--------------------+\n");
   printf("   | Idx  | Red   | Green | Blue  | Alpha |     Color Name     |    Color Title     |\n");
   printf("   +------+-------+-------+-------+-------+--------------------+--------------------+\n");

   for (Int_t i = ci1; i<ci2; i++) {
      color = (TColor*)colors->At(i);
      if (color) {
         printf("   | %4d | %5.3f | %5.3f | %5.3f | %5.3f | %18s | %18s |\n", i,
                                                           color->GetRed(),
                                                           color->GetGreen(),
                                                           color->GetBlue(),
                                                           color->GetAlpha(),
                                                           color->GetName(),
                                                           color->GetTitle());
         nc++;
      } else {
         printf("   | %4d |   ?   |   ?   |   ?   |   ?   |                  ? |                  ? |\n",i);
      }
   }
   printf("   +------+-------+-------+-------+-------+--------------------+--------------------+\n");
   printf("   | Number of possible colors = %4d                                               |\n",ncolors);
   printf("   | Number of defined colors between %4d and %4d = %4d                          |\n",ci1,ci2,nc);
   printf("   | Number of free indeces between %4d and %4d   = %4d                          |\n",ci1,ci2,ci2-ci1-nc);
   printf("   +--------------------------------------------------------------------------------+\n\n");

}