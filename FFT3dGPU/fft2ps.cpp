#include "fft2ps.h"
#include "core/Debug class.h"
const float pi(acos(-1.0));

psButterfly::psButterfly(LPDIRECT3DDEVICE9 pDevice, D3DXMACRO *defs, LPCSTR functionname) :
  Pixelshader(pDevice, SRC_SHADER, functionname, D3DXGetPixelShaderProfile(pDevice), defs) {}

psButterflyHSP::psButterflyHSP(LPDIRECT3DDEVICE9 pDevice, int xn, int x, int yn, int y) :
  psButterfly(pDevice, VecToMacroArray(D3DXVECTOR2(1 / (float)(xn * 2), 0), "LOADOFFSET", CreateMacroArray(1)), "Butterfly"),
  sSrc(_pConstantTable->GetConstantByName(NULL, "Src0")),
  vs(pDevice, SRC_SHADER, "fft2", D3DXGetVertexShaderProfile(pDevice), 0),
  vs0(pDevice, SRC_SHADER, "fft0", D3DXGetVertexShaderProfile(pDevice), 0),
  ps0(pDevice, SRC_SHADER, "Butterfly", D3DXGetPixelShaderProfile(pDevice), SetMacroArray("STAGE0", VecToMacroArray(D3DXVECTOR2(1 / (float)(xn * 2), 0), "LOADOFFSET", CreateMacroArray(2), 1))),
  q0(pDevice, x, xn, y, yn, true),
  dir(vs._pConstantTable->GetConstantByName(NULL, "dir"))
{

  q = x > 2 ? NEW FFTQuad2(pDevice, x, xn, y, yn, true, 1) : 0;
  quad = q;
}

HRESULT psButterflyHSP::Apply(Texture* src1, Texture* src2, TextureRT *dst1, TextureRT *dst2, int stage, bool forward)
{
  PROFILE_BLOCK
    if (stage == 1)
    {
      V_RETURN(SetActive());
      V_RETURN(quad->SetActive());
      V_RETURN(vs.SetActive());
    }
  V_RETURN(dst1->SetAsRenderTarget(0));
  V_RETURN(dst2->SetAsRenderTarget(1));
  V_RETURN(src1->SetAsTexture(GetSamplerIndex(sSrc)));
  V_RETURN(q->DrawOdd(stage));
  V_RETURN(src2->SetAsTexture(GetSamplerIndex(sSrc)));
  return q->DrawEven(stage);
}

//stage 0
HRESULT psButterflyHSP::Apply(Texture* src, TextureRT *dst1, TextureRT *dst2, bool forward)
{
  PROFILE_BLOCK
    V_RETURN(q0.SetActive());
  V_RETURN(vs0.SetActive());
  V_RETURN(ps0.SetActive());
  V_RETURN(dst1->SetAsRenderTarget(0));
  V_RETURN(dst2->SetAsRenderTarget(1));
  V_RETURN(src->SetAsTexture(GetSamplerIndex(sSrc)));
  float sign = forward ? 1 : -1;
  V_RETURN(vs0._pConstantTable->SetFloat(_pDevice, dir, sign));
  return q0.Draw();
}
//****
psButterflyHMP::psButterflyHMP(LPDIRECT3DDEVICE9 pDevice, int xn, int x, int yn, int y) :
  psButterfly(pDevice, VecToMacroArray(D3DXVECTOR2(1 / (float)(xn * 2), 0), "LOADOFFSET", SetMacroArray("MULTIPASS", SetMacroArray("PASS0", CreateMacroArray(3), 2), 1)), "Butterfly"),
  sSrc(_pConstantTable->GetConstantByName(NULL, "Src0")),
  vs(pDevice, SRC_SHADER, "fft2", D3DXGetVertexShaderProfile(pDevice), 0),
  vs0(pDevice, SRC_SHADER, "fft0", D3DXGetVertexShaderProfile(pDevice), 0),
  ps0_pass0(pDevice, SRC_SHADER, "Butterfly", D3DXGetPixelShaderProfile(pDevice), SetMacroArray("STAGE0", VecToMacroArray(D3DXVECTOR2(1 / (float)(xn * 2), 0), "LOADOFFSET", SetMacroArray("MULTIPASS", SetMacroArray("PASS0", CreateMacroArray(4), 3), 2), 1))),
  ps0_pass1(pDevice, SRC_SHADER, "Butterfly", D3DXGetPixelShaderProfile(pDevice), SetMacroArray("STAGE0", VecToMacroArray(D3DXVECTOR2(1 / (float)(xn * 2), 0), "LOADOFFSET", SetMacroArray("MULTIPASS", CreateMacroArray(3), 2), 1))),
  ps_pass1(pDevice, SRC_SHADER, "Butterfly", D3DXGetPixelShaderProfile(pDevice), VecToMacroArray(D3DXVECTOR2(1 / (float)(xn * 2), 0), "LOADOFFSET", SetMacroArray("MULTIPASS", CreateMacroArray(2), 1))),
  q0(pDevice, x, xn, y, yn, true),
  dir(vs._pConstantTable->GetConstantByName(NULL, "dir"))
{
  q = NEW FFTQuad2(pDevice, x, xn, y, yn, true, 1);
  quad = q;
}

HRESULT psButterflyHMP::Apply(Texture* src1, Texture* src2, TextureRT *dst1, TextureRT *dst2, int stage, bool forward)
{
  PROFILE_BLOCK
    if (stage == 1)
    {

      V_RETURN(quad->SetActive());
      V_RETURN(vs.SetActive());
    }
  V_RETURN(SetActive());
  V_RETURN(dst1->SetAsRenderTarget(0));
  V_RETURN(src1->SetAsTexture(GetSamplerIndex(sSrc)));
  V_RETURN(q->DrawOdd(stage));
  V_RETURN(src2->SetAsTexture(GetSamplerIndex(sSrc)));
  V_RETURN(q->DrawEven(stage));

  V_RETURN(ps_pass1.SetActive());
  V_RETURN(dst2->SetAsRenderTarget(0));
  V_RETURN(src1->SetAsTexture(GetSamplerIndex(sSrc)));
  V_RETURN(q->DrawOdd(stage));
  V_RETURN(src2->SetAsTexture(GetSamplerIndex(sSrc)));
  return q->DrawEven(stage);
}

//stage 0
HRESULT psButterflyHMP::Apply(Texture* src, TextureRT *dst1, TextureRT *dst2, bool forward)
{
  PROFILE_BLOCK
    V_RETURN(q0.SetActive());
  V_RETURN(vs0.SetActive());
  V_RETURN(ps0_pass0.SetActive());
  V_RETURN(dst1->SetAsRenderTarget(0));

  V_RETURN(src->SetAsTexture(GetSamplerIndex(sSrc)));
  float sign = forward ? 1 : -1;
  V_RETURN(vs0._pConstantTable->SetFloat(_pDevice, dir, sign));
  V_RETURN(q0.Draw());

  V_RETURN(ps0_pass1.SetActive());
  V_RETURN(dst2->SetAsRenderTarget(0));
  V_RETURN(src->SetAsTexture(GetSamplerIndex(sSrc)));
  return q0.Draw();
}
//**********************************************************************************


psButterflyVSP::psButterflyVSP(LPDIRECT3DDEVICE9 pDevice, int xn, int x, int yn, int y) :
  psButterfly(pDevice, VecToMacroArray(D3DXVECTOR2(0, 1 / (float)(yn * 2)), "LOADOFFSET", CreateMacroArray(1)), "Butterfly"),
  sSrc(_pConstantTable->GetConstantByName(NULL, "Src0")),
  vs(pDevice, SRC_SHADER, "fft2", D3DXGetVertexShaderProfile(pDevice), 0),
  vs0(pDevice, SRC_SHADER, "fft0", D3DXGetVertexShaderProfile(pDevice), 0),
  ps0(pDevice, SRC_SHADER, "Butterfly", D3DXGetPixelShaderProfile(pDevice), SetMacroArray("STAGE0", VecToMacroArray(D3DXVECTOR2(0, 1 / (float)(yn * 2)), "LOADOFFSET", CreateMacroArray(2), 1))),
  q0(pDevice, x, xn, y, yn, false),
  dir(vs._pConstantTable->GetConstantByName(NULL, "dir"))
{
  q = NEW FFTQuad2(pDevice, x, xn, y, yn, false, 1);
  quad = q;
}

HRESULT psButterflyVSP::Apply(Texture* src1, Texture* src2, TextureRT *dst1, TextureRT *dst2, int stage, bool forward)
{
  PROFILE_BLOCK
    if (stage == 1)
    {
      V_RETURN(quad->SetActive());
      V_RETURN(vs.SetActive());
      V_RETURN(SetActive());
    }
  V_RETURN(dst1->SetAsRenderTarget(0));
  V_RETURN(dst2->SetAsRenderTarget(1));
  V_RETURN(src1->SetAsTexture(GetSamplerIndex(sSrc)));
  V_RETURN(q->DrawOdd(stage));
  V_RETURN(src2->SetAsTexture(GetSamplerIndex(sSrc)));
  return q->DrawEven(stage);
}

//stage 0
HRESULT psButterflyVSP::Apply(Texture* src, TextureRT *dst1, TextureRT *dst2, bool forward)
{
  PROFILE_BLOCK
    V_RETURN(q0.SetActive());
  V_RETURN(vs0.SetActive());
  V_RETURN(ps0.SetActive());
  V_RETURN(dst1->SetAsRenderTarget(0));
  V_RETURN(dst2->SetAsRenderTarget(1));
  V_RETURN(src->SetAsTexture(GetSamplerIndex(sSrc)));
  float sign = forward ? 1 : -1;
  V_RETURN(vs0._pConstantTable->SetFloat(_pDevice, dir, sign));
  return q0.Draw();
}
//****
psButterflyVMP::psButterflyVMP(LPDIRECT3DDEVICE9 pDevice, int xn, int x, int yn, int y) :
  psButterfly(pDevice, VecToMacroArray(D3DXVECTOR2(0, 1 / (float)(yn * 2)), "LOADOFFSET", SetMacroArray("MULTIPASS", SetMacroArray("PASS0", CreateMacroArray(3), 2), 1)), "Butterfly"),
  sSrc(_pConstantTable->GetConstantByName(NULL, "Src0")),
  vs(pDevice, SRC_SHADER, "fft2", D3DXGetVertexShaderProfile(pDevice), 0),
  vs0(pDevice, SRC_SHADER, "fft0", D3DXGetVertexShaderProfile(pDevice), 0),
  ps0_pass0(pDevice, SRC_SHADER, "Butterfly", D3DXGetPixelShaderProfile(pDevice), SetMacroArray("STAGE0", VecToMacroArray(D3DXVECTOR2(0, 1 / (float)(yn * 2)), "LOADOFFSET", SetMacroArray("MULTIPASS", SetMacroArray("PASS0", CreateMacroArray(4), 3), 2), 1))),
  ps0_pass1(pDevice, SRC_SHADER, "Butterfly", D3DXGetPixelShaderProfile(pDevice), SetMacroArray("STAGE0", VecToMacroArray(D3DXVECTOR2(0, 1 / (float)(yn * 2)), "LOADOFFSET", SetMacroArray("MULTIPASS", CreateMacroArray(3), 2), 1))),
  ps_pass1(pDevice, SRC_SHADER, "Butterfly", D3DXGetPixelShaderProfile(pDevice), VecToMacroArray(D3DXVECTOR2(0, 1 / (float)(yn * 2)), "LOADOFFSET", SetMacroArray("MULTIPASS", CreateMacroArray(2), 1))),
  q0(pDevice, x, xn, y, yn, false),
  dir(vs._pConstantTable->GetConstantByName(NULL, "dir"))
{
  q = NEW FFTQuad2(pDevice, x, xn, y, yn, false, 1);
  quad = q;
}

HRESULT psButterflyVMP::Apply(Texture* src1, Texture* src2, TextureRT *dst1, TextureRT *dst2, int stage, bool forward)
{
  PROFILE_BLOCK
    if (stage == 1)
    {

      V_RETURN(quad->SetActive());
      V_RETURN(vs.SetActive());
    }
  V_RETURN(SetActive());
  V_RETURN(dst1->SetAsRenderTarget(0));
  V_RETURN(src1->SetAsTexture(GetSamplerIndex(sSrc)));
  V_RETURN(q->DrawOdd(stage));
  V_RETURN(src2->SetAsTexture(GetSamplerIndex(sSrc)));
  V_RETURN(q->DrawEven(stage));

  V_RETURN(ps_pass1.SetActive());
  V_RETURN(dst2->SetAsRenderTarget(0));
  V_RETURN(src1->SetAsTexture(GetSamplerIndex(sSrc)));
  V_RETURN(q->DrawOdd(stage));
  V_RETURN(src2->SetAsTexture(GetSamplerIndex(sSrc)));
  return q->DrawEven(stage);
}

//stage 0
HRESULT psButterflyVMP::Apply(Texture* src, TextureRT *dst1, TextureRT *dst2, bool forward)
{
  PROFILE_BLOCK
    V_RETURN(q0.SetActive());
  V_RETURN(vs0.SetActive());
  V_RETURN(ps0_pass0.SetActive());
  V_RETURN(dst1->SetAsRenderTarget(0));

  V_RETURN(src->SetAsTexture(GetSamplerIndex(sSrc)));
  float sign = forward ? 1 : -1;
  V_RETURN(vs0._pConstantTable->SetFloat(_pDevice, dir, sign));
  V_RETURN(q0.Draw());

  V_RETURN(ps0_pass1.SetActive());
  V_RETURN(dst2->SetAsRenderTarget(0));
  V_RETURN(src->SetAsTexture(GetSamplerIndex(sSrc)));
  return q0.Draw();
}
//******************************************
psC2Rfft::psC2Rfft(LPDIRECT3DDEVICE9 pDevice, int xn, int x, int yn, int y) :
  Pixelshader(pDevice, SRC_SHADER, "C2Rfft_ps", D3DXGetPixelShaderProfile(pDevice)),
  sSrc1(_pConstantTable->GetConstantByName(NULL, "Src0")),
  sSrc2(_pConstantTable->GetConstantByName(NULL, "Src1")),
  t00(_pConstantTable->GetConstantByName(NULL, "t00")),
  t11(_pConstantTable->GetConstantByName(NULL, "t11")),
  vs(pDevice, SRC_SHADER, "passthough3", D3DXGetVertexShaderProfile(pDevice), 0)
{
  q = NEW C2RQuad(pDevice, x, xn, y, yn);
  quad = q;
}


HRESULT psC2Rfft::Apply(Texture* src1, Texture* src2, TextureRT *dst)
{
  PROFILE_BLOCK
    V_RETURN(quad->SetActive());
  V_RETURN(vs.SetActive());
  V_RETURN(SetActive());
  V_RETURN(_pConstantTable->SetBool(_pDevice, t00, true));
  V_RETURN(dst->SetAsRenderTarget(0));
  V_RETURN(_pDevice->SetRenderTarget(1, 0));
  V_RETURN(src1->SetAsTexture(GetSamplerIndex(sSrc1)));
  V_RETURN(q->Draw00());
  V_RETURN(src2->SetAsTexture(GetSamplerIndex(sSrc2)));
  V_RETURN(_pConstantTable->SetBool(_pDevice, t00, false));
  V_RETURN(_pConstantTable->SetBool(_pDevice, t11, false));
  V_RETURN(q->Draw01());
  V_RETURN(src2->SetAsTexture(GetSamplerIndex(sSrc1)));
  V_RETURN(_pConstantTable->SetBool(_pDevice, t11, true));
  V_RETURN(q->Draw11());
  V_RETURN(src1->SetAsTexture(GetSamplerIndex(sSrc2)));
  V_RETURN(_pConstantTable->SetBool(_pDevice, t11, false));
  return q->Draw10();
}


psCombine::psCombine(LPDIRECT3DDEVICE9 pDevice, int xn, int x, int yn, int y, bool horizontal, float norm) :
  Pixelshader(pDevice, SRC_SHADER, "Combine", D3DXGetPixelShaderProfile(pDevice), FloatToMacroArray(norm, "NORM", CreateMacroArray(1))),
  sSrc(_pConstantTable->GetConstantByName(NULL, "Src0")),
  vs(pDevice, SRC_SHADER, "passthough", D3DXGetVertexShaderProfile(pDevice), 0)
{
  q = NEW CollectQuad(pDevice, x, xn, y, yn, horizontal);
  quad = q;
}

HRESULT psCombine::Apply(Texture* src1, Texture* src2, TextureRT *dst)
{
  PROFILE_BLOCK
    V_RETURN(quad->SetActive());
  V_RETURN(vs.SetActive());
  //_pDevice->SetVertexShader(0);
  V_RETURN(SetActive());
  V_RETURN(_pDevice->SetRenderTarget(1, 0));
  V_RETURN(dst->SetAsRenderTarget(0));
  V_RETURN(src1->SetAsTexture(GetSamplerIndex(sSrc)));
  V_RETURN(q->DrawOdd());
  V_RETURN(src2->SetAsTexture(GetSamplerIndex(sSrc)));
  return q->DrawEven();
}

psR2Cfft::psR2Cfft(LPDIRECT3DDEVICE9 pDevice, int xn, int x, int yn, int y) :
  Pixelshader(pDevice, SRC_SHADER, "R2Cfft", D3DXGetPixelShaderProfile(pDevice)),
  sSrc(_pConstantTable->GetConstantByName(NULL, "Src0")),
  vs(pDevice, SRC_SHADER, "passthough3", D3DXGetVertexShaderProfile(pDevice), 0)
{
  q = NEW R2CFFTQuad(pDevice, x, xn, y, yn);
  quad = q;
}

HRESULT psR2Cfft::Apply(Texture* src1, Texture* src2, TextureRT *dst)
{
  PROFILE_BLOCK
    V_RETURN(quad->SetActive());
  V_RETURN(vs.SetActive());
  //_pDevice->SetVertexShader(0);
  V_RETURN(SetActive());
  V_RETURN(_pDevice->SetRenderTarget(1, 0));
  V_RETURN(dst->SetAsRenderTarget(0));
  V_RETURN(src1->SetAsTexture(GetSamplerIndex(sSrc)));
  V_RETURN(q->DrawOdd());
  V_RETURN(src2->SetAsTexture(GetSamplerIndex(sSrc)));
  return q->DrawEven();
}