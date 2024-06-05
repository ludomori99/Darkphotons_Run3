#include "TStyle.h"

void CMS(TPad* pad,string lumi="62.4", string com="13.6", float extra_factor=1.){
    float H = pad->GetWh();
    float W = pad->GetWw();
    float l = pad->GetLeftMargin();
    float t = pad->GetTopMargin();
    float r = pad->GetRightMargin();
    float b = pad->GetBottomMargin();
    float e = 0.07;

    float relPosX    = 0.045;
    float relPosY    = 0.045;
    float relExtraDY = 1.2;

    float posX_ =   l + relPosX*(1-l-r);
    float posY_ = 1-t - relPosY*(1-t-b);
    
    int alignY_=1;
    int alignX_=1;
    int align_ = 10*alignX_ + alignY_;


    TString cmsText     = "CMS";
    float cmsTextFont   = 61;  // default is helvetic-bold

    bool writeExtraText = true;
    TString extraText   = "Preliminary";
    float extraTextFont = 52;  // default is helvetica-italics
    
    TString lumiText = lumi + " fb^{-1} (" + com + " TeV)";

    float lumiTextSize     = 1.15;
    float lumiTextOffset   = -0.15;
    float cmsTextSize      = 1.35;
    float cmsTextOffset    = 0.1;  

    TLatex latex;
    latex.SetNDC();
    latex.SetTextAngle(0);
    latex.SetTextColor(kBlack);    

    float extraOverCmsTextSize  = 0.76;

    float extraTextSize = extraOverCmsTextSize*cmsTextSize;


    latex.SetTextFont(42);
    latex.SetTextAlign(31); 
    latex.SetTextSize(extra_factor*lumiTextSize*t);    
    latex.DrawLatex(1-r,1-t+lumiTextOffset*t,lumiText);

    latex.SetTextFont(cmsTextFont);
    latex.SetTextAlign(11); 
    latex.SetTextSize(extra_factor*cmsTextSize*t);    
    latex.DrawLatex(l,1-t+lumiTextOffset*t,cmsText);

    latex.SetTextFont(extraTextFont);
    latex.SetTextSize(extra_factor*extraTextSize*t);
    latex.SetTextAlign(align_);
    latex.DrawLatex(posX_+e, 1-t+lumiTextOffset*t, extraText);      
    return;
}

void CMS_single(TPad* pad,string lumi="62.4", string com="13.6", float extra_factor=1.){
    float H = pad->GetWh();
    float W = pad->GetWw();
    float l = pad->GetLeftMargin();
    float t = pad->GetTopMargin();
    float r = pad->GetRightMargin();
    float b = pad->GetBottomMargin();
    float e = 0.05;

    float relPosX    = 0.045;
    float relPosY    = 0.045;
    float relExtraDY = 1.2;

    float posX_ =   l + relPosX*(1-l-r);
    float posY_ = 1-t - relPosY*(1-t-b);
    
    int alignY_=1;
    int alignX_=1;
    int align_ = 10*alignX_ + alignY_;


    TString cmsText     = "CMS";
    float cmsTextFont   = 61;  // default is helvetic-bold

    bool writeExtraText = true;
    TString extraText   = "Preliminary";
    float extraTextFont = 52;  // default is helvetica-italics
    
    TString lumiText = lumi + " fb^{-1} (" + com + " TeV)";

    float lumiTextSize     = 1.15;
    float lumiTextOffset   = 0.2;
    float cmsTextSize      = 1.35;
    float cmsTextOffset    = 0.1;  

    TLatex latex;
    latex.SetNDC();
    latex.SetTextAngle(0);
    latex.SetTextColor(kBlack);    

    float extraOverCmsTextSize  = 0.76;

    float extraTextSize = extraOverCmsTextSize*cmsTextSize;

    latex.SetTextFont(42);
    latex.SetTextAlign(31); 
    latex.SetTextSize(extra_factor*lumiTextSize*t);    
    latex.DrawLatex(1-r,1-t+lumiTextOffset*t,lumiText);

    latex.SetTextFont(cmsTextFont);
    latex.SetTextAlign(11); 
    latex.SetTextSize(extra_factor*cmsTextSize*t);    
    latex.DrawLatex(l,1-t+lumiTextOffset*t,cmsText);

    latex.SetTextFont(extraTextFont);
    latex.SetTextSize(extra_factor*extraTextSize*t);
    latex.SetTextAlign(align_);
    latex.DrawLatex(posX_+e, 1-t+lumiTextOffset*t, extraText);      
    return;
}