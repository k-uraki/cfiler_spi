#ifndef __SPI_H__
#define __SPI_H__

#include "windows.h"
//#include <vector>
#include <string>

//---- from pythonutil.h���痬�p
#include "python.h"

namespace PythonUtil
{
    bool PyStringToString( const PyObject * pystr, std::string * str );
	bool PyStringToWideString( const PyObject * pystr, std::wstring * str );
};
//---- from pythonutil.h���痬�p


// SPI�摜���
struct PictureInfo
{
	long left,top;		// �摜��W�J����ʒu
	long width;			// �摜�̕�(pixel)
	long height;		// �摜�̍���(pixel)
	WORD x_density;		// ��f�̐����������x
	WORD y_density;		// ��f�̐����������x
	short colorDepth;	// �P��f�������bit��
	HLOCAL hInfo;		// �摜���̃e�L�X�g���
};

// SPI����N���X
// 1�C���X�^���X��1��SPI�ɑΉ�����
class CSPI
{
public:
	CSPI();
	~CSPI();
	
	int loadDll(const WCHAR *dll_fname);
	
	void term();
	
	void AddRef() { ref_count++; /* printf("Image::AddRef : %d\n", ref_count ); */ }
	void Release() { ref_count--; /* printf("Image::Release : %d\n", ref_count ); */ if(ref_count==0) delete this; }
	
	// GetPluginInfo�n
	bool getPluginInfoAPIVer(std::string *s);
	bool getPluginInfoAbout (std::string *s);
	bool getPluginInfoExt   (int no, std::string *s);	// no=0�`
	
	// IsSupported�n
	bool isSupported(LPCSTR filename);
	bool isSupportedMem(const void *fimg);
	
	// GetPictureInfo�n
	bool getPictureInfo(LPCSTR filename, PictureInfo *info);
	bool getPictureInfoMem(const void *fimg, unsigned long size, PictureInfo *info);
	
	// GetPicture�n
	bool getPicture(LPCSTR filename, HLOCAL *hbminfo, HLOCAL *hbm);
	bool getPictureMem(const void *fimg, unsigned long size, HLOCAL *hbminfo, HLOCAL *hbm);
	
private:
	int	ref_count;
	
	HMODULE	_hDll;
	
	// ToDo: 00AM�v���O�C���Ή� �����i�������Ɨv��Ȃ����ȁc
/*
  �EGetPluginInfo - Plug-in�Ɋւ�����𓾂�

    Prototype:
      extern "C" int _export PASCAL GetPluginInfo(int infono,
                                                LPSTR buf,int buflen);
    Parameter:
      int infono : �擾������ԍ�
                      0   : Plug-in API�o�[�W����
                      1   : Plug-in���A�o�[�W�����y�� copyright
                            (Susie��About..�ɕ\������܂�)
                      2n+2: ��\�I�Ȋg���q ("*.JPG" "*.RGB;*.Q0" �Ȃ�)
                      2n+3: �t�@�C���`����
                            (�t�@�C���^�C�v���Ƃ��Ďg���܂�)
      LPSTR buf    : ����[�߂�o�b�t�@
      int buflen : �o�b�t�@��(byte)

    Return:
      �o�b�t�@�ɏ������񂾕�������Ԃ��܂��B
      ���ԍ��������̏ꍇ�A0��Ԃ��܂��B

    ���:
      ���ԍ�0��1�͂��ׂẴo�[�W�����ŋ��ʂƂ��܂��B
      2�ȍ~�͓�Âg�݂�Susie��OPEN�_�C�A���O�ŗp������ł��B
      ���plug-in�ŕ����̉摜�t�H�[�}�b�g�ɑΉ����Ă���ꍇ��
      ���̐������g���q�ƃt�@�C���`������p�ӂ��܂��B
*/
	typedef int (PASCAL *GET_PLUGIN_INFO)(int , LPSTR , int );
	GET_PLUGIN_INFO	_funcGetPluginInfo;
	
/*
  �EIsSupported - �W�J�\��(�Ή����Ă���)�t�@�C���`�������ׂ�B
    Prototype:
      extern "C" int _export PASCAL IsSupported(LPSTR filename,DWORD dw);

    Parameter:
      LPSTR filename : �t�@�C���l�[��
      DWORD dw       : ��ʃ��[�h��  0  �̂Ƃ�:
                           �t�@�C���n���h��
                       ��ʃ��[�h�� ��0 �̂Ƃ�:
                           �t�@�C���擪��(2Kbyte�ȏ�)��ǂݍ��񂾃o�b�t�@�ւ�
                           �|�C���^
                           �t�@�C���T�C�Y��2Kbyte�ȉ��̏ꍇ���o�b�t�@��2Kbyte
                           �m�ۂ��A�]���� 0 �Ŗ��߂邱��

    Return:
      �Ή����Ă���摜�t�H�[�}�b�g�ł���Δ�0��Ԃ�

    ���:
      �ePlug-in�͊�{�I�ɓn���ꂽ�t�@�C���̃w�b�_�𒲂ׁA�����̑Ή������t�@�C��
      �t�H�[�}�b�g�ł��邩�ǂ����𒲂ׂ�B
      �܂�Ƀt�@�C����(�g���q)�𔻒f�ޗ��Ƃ��ĕK�v�Ƃ�����A�����̃t�@�C����
      �\������Ă���ꍇ������̂ŁA�t�@�C����(�t���p�X)�������ɉ������B
      ����z�z��Plug-in�ł�filename�͎g���Ă��Ȃ��B
*/
	typedef int (PASCAL *IS_SUPPORTED)(LPSTR ,DWORD );
	IS_SUPPORTED	_funcIsSupported;
	
/*
  �EGetPictureInfo - �摜�t�@�C���Ɋւ�����𓾂�
    Prototype:
      extern "C" int _export PASCAL GetPictureInfo(
         LPSTR buf,long len,unsigned int flag,struct PictureInfo *lpInfo);

    Parameter:
      LPSTR buf : ���͂��t�@�C���̏ꍇ �t�@�C����
                        �������[�̏ꍇ �t�@�C���C���[�W�ւ̃|�C���^
      long len  : ���͂��t�@�C���̏ꍇ �Ǎ��݊J�n�I�t�Z�b�g(MacBin�Ή��̂���)
                        �������[�̏ꍇ �f�[�^�T�C�Y
      unsigned int flag : �ǉ���� xxxx xxxx xxxx xSSS
                  SSS : ���͌`��
                      000 : �f�B�X�N�t�@�C��
                      001 : ��������̃C���[�W
      struct PictureInfo *lpInfo :
                  struct PictureInfo
                  {
                    long left,top;    �摜��W�J����ʒu
                    long width;       �摜�̕�(pixel)
                    long height;      �摜�̍���(pixel)
                    WORD x_density;   ��f�̐����������x
                    WORD y_density;   ��f�̐����������x
                    short colorDepth; �P��f�������bit��
                    HLOCAL hInfo;    �摜���̃e�L�X�g���
                  };
                  hInfo�ɂ͕K�v�ɉ�����Plug-in���m�ۂ���Global�������[��
                  �n���h�����i�[�����B

    Return:
      �G���[�R�[�h�B0�Ȃ琳��I���B
*/
	typedef int (PASCAL *GET_PICTURE_INFO)(LPSTR ,long ,unsigned int ,PictureInfo *lpInfo);
	GET_PICTURE_INFO	_funcGetPictureInfo;
	
/*
  �EGetPicture - �摜��W�J����
    Prototype:
      extern "C" int _export PASCAL GetPicture(
              LPSTR buf,long len,unsigned int flag,
              HANDLE *pHBInfo,HANDLE *pHBm,
              FARPROC lpPrgressCallback,long lData);

    Parameter:
      LPSTR buf : ���͂��t�@�C���̏ꍇ �t�@�C����
                        �������[�̏ꍇ �t�@�C���C���[�W�ւ̃|�C���^
      long len  : ���͂��t�@�C���̏ꍇ �Ǎ��݊J�n�I�t�Z�b�g(MacBin�Ή��̂���)
                        �������[�̏ꍇ �f�[�^�T�C�Y
      unsigned int flag : �ǉ���� xxxx xxxx xxxx xSSS
                  SSS : ���͌`��
                      000 : �f�B�X�N�t�@�C��
                      001 : ��������̃C���[�W
      HLOCAL *pHBInfo : BITMAPINFO �\���̂��[�߂�ꂽ�������n���h����
                             �Ԃ����B
      HLOCAL *pHBm    : �r�b�g�}�b�v�f�[�^�{�̂̃������n���h�����Ԃ����
      FARPROC lpPrgressCallback :
                �r���o�߂�\������R�[���o�b�N�֐��ւ̃|�C���^�B
                NULL�̏ꍇ�Aplug-in�͏������I������܂Ńv���Z�X���L���A
                ���f���o���܂���B
                �R�[���o�b�N�֐���prototype:
                  int PASCAL ProgressCallback(
                                        int nNum,int nDenom,long lData);
                  �܂� nNum==0 �ŃR�[������AnNum==nDenom �ɂȂ�܂�
                  ����I�ɌĂ΂��B
                  �ߒl�� ��0 �̎��APlug-in�͏����𒆒f����B
      long lData : �R�[���o�b�N�֐��ɓn��long�f�[�^�B
                  �|�C���^����K�v�ɉ����Ď󂯓n����B

    Return:
      �G���[�R�[�h�B0�Ȃ琳��I���B

    ���:
      �v���O�C����LocalAlloc�ɂ���ĕK�v�ȃ������[���m�ۂ��A���̃n���h����
      �Ԃ��B
      �A�v���P�[�V������LocalFree�ɂ���ă������[���J������K�v������B
*/
	typedef int (PASCAL *GET_PICTURE)(LPSTR ,long ,unsigned int ,HANDLE *,HANDLE *,FARPROC ,long );
	GET_PICTURE	_funcGetPicture;
	
/*
  �EGetPreview - �v���r���[�E�J�^���O�\���p�摜�k���W�J���[�e�B��
    Prototype:
      extern "C" int _export PASCAL GetPreview(
              LPSTR buf,long len,unsigned int flag,
              HANDLE *pHBInfo,HANDLE *pHBm,
              FARPROC lpPrgressCallback,long lData);

    Parameter:
      GETPICTURE�Q�ƁB

    Return:
      �G���[�R�[�h�B0�Ȃ琳��I���B
      ���̊֐��̓I�v�V�����ł���A���Ή��̏ꍇ�� -1 ��Ԃ��B

    ���:
      �v���r���[���ŗp����k�����ꂽ�摜���t�@�C������쐬����B
      JPEG�̗l�ɁA�A���S���Y���̊֌W�ŏk�����ꂽ�T�C�Y�ł͍����ɓW�J�o����
      �Ƃ��ɂ��̊֐����C���v�������g����B
      ����z�z��Plug-in�ł� IFJPEG.PLG �̂ݑΉ�(1/4�T�C�Y�œW�J)���Ă���B
      ���Ή��̏ꍇ�ASusie�͒ʏ�̓W�J���[�e�B����p���ēW�J������
      �k���������s���B
      �i�Ή����Ă��Ă��k�����[�h���ꂽ�摜���X�ɃT�C�Y�������Ă���j
      �v���O�C����LocalAlloc�ɂ���ĕK�v�ȃ������[���m�ۂ��A���̃n���h����
      �Ԃ��B
      �A�v���P�[�V������LocalFree�ɂ���ă������[���J������K�v������B

  �E�G���[�R�[�h
    0 : ����I��
   -1 : ���̋@�\�̓C���v�������g����Ă��Ȃ�
    1 : �R�[���o�b�N�֐�����0��Ԃ����̂œW�J�𒆎~����
    2 : ���m�̃t�H�[�}�b�g
    3 : �f�[�^�����Ă���
    4 : �������[���m�ۏo���Ȃ�
    5 : �������[�G���[�iLock�o���Ȃ��A���j
    6 : �t�@�C�����[�h�G���[
    7 : �i�\��j
    8 : �����G���[
*/
	// �ȗ�
};

// Python�C���^�[�t�F�[�X
extern PyTypeObject SPI_Type;
#define SPI_Check(op) PyObject_TypeCheck(op, &SPI_Type)

struct SPI_Object
{
    PyObject_HEAD
    CSPI * p;
};




#endif	// __SPI_H__

/*
Susie 32bit Plug-in �d�l rev5

�P�D�͂��߂�
  Susie 32bit Plug-in �� Windows �� DLL �ł���A��q�� API �ɂ�� Susie �ȊO��
  �\�t�g�E�F�A������ȒP�Ɏg�������o���܂��B
  �܂��A���̎d�l�ʂ��Plug-in������ Susie ��V�����摜�t�H�[�}�b�g��
  �Ή������鎖���\�ł��B

�Q�DPlug-in API�̃o�[�W����
  ����̊g�������������邽�߁APlug-in��API�̃o�[�W�����ԍ������܂��B
  ���̃o�[�W�����ԍ��͂��ׂẴo�[�W�����ɋ��ʂł���֐�'GETPLUGININFO'�ɂ����
  �擾�o���܂��B
  �o�[�W�����ԍ��͊�{�I��4byte�̃R�[�h�ňȉ��̈Ӗ��������܂��B

      00 I N
      ~T T T
       | | +-- N : Normal,  M : Multi-picture
       | +---- I : Import filter, X : Export filter, A : Archive extractor
       +------ Virsion No.

�R�D���ʊ֐�
  �EGetPluginInfo - Plug-in�Ɋւ�����𓾂�

    Prototype:
      extern "C" int _export PASCAL GetPluginInfo(int infono,
                                                LPSTR buf,int buflen);
    Parameter:
      int infono : �擾������ԍ�
                      0   : Plug-in API�o�[�W����
                      1   : Plug-in���A�o�[�W�����y�� copyright
                            (Susie��About..�ɕ\������܂�)
                      2n+2: ��\�I�Ȋg���q ("*.JPG" "*.RGB;*.Q0" �Ȃ�)
                      2n+3: �t�@�C���`����
                            (�t�@�C���^�C�v���Ƃ��Ďg���܂�)
      LPSTR buf    : ����[�߂�o�b�t�@
      int buflen : �o�b�t�@��(byte)

    Return:
      �o�b�t�@�ɏ������񂾕�������Ԃ��܂��B
      ���ԍ��������̏ꍇ�A0��Ԃ��܂��B

    ���:
      ���ԍ�0��1�͂��ׂẴo�[�W�����ŋ��ʂƂ��܂��B
      2�ȍ~�͓�Âg�݂�Susie��OPEN�_�C�A���O�ŗp������ł��B
      ���plug-in�ŕ����̉摜�t�H�[�}�b�g�ɑΉ����Ă���ꍇ��
      ���̐������g���q�ƃt�@�C���`������p�ӂ��܂��B

�S�D'00IN'�̊֐�
  �EIsSupported - �W�J�\��(�Ή����Ă���)�t�@�C���`�������ׂ�B
    Prototype:
      extern "C" int _export PASCAL IsSupported(LPSTR filename,DWORD dw);

    Parameter:
      LPSTR filename : �t�@�C���l�[��
      DWORD dw       : ��ʃ��[�h��  0  �̂Ƃ�:
                           �t�@�C���n���h��
                       ��ʃ��[�h�� ��0 �̂Ƃ�:
                           �t�@�C���擪��(2Kbyte�ȏ�)��ǂݍ��񂾃o�b�t�@�ւ�
                           �|�C���^
                           �t�@�C���T�C�Y��2Kbyte�ȉ��̏ꍇ���o�b�t�@��2Kbyte
                           �m�ۂ��A�]���� 0 �Ŗ��߂邱��

    Return:
      �Ή����Ă���摜�t�H�[�}�b�g�ł���Δ�0��Ԃ�

    ���:
      �ePlug-in�͊�{�I�ɓn���ꂽ�t�@�C���̃w�b�_�𒲂ׁA�����̑Ή������t�@�C��
      �t�H�[�}�b�g�ł��邩�ǂ����𒲂ׂ�B
      �܂�Ƀt�@�C����(�g���q)�𔻒f�ޗ��Ƃ��ĕK�v�Ƃ�����A�����̃t�@�C����
      �\������Ă���ꍇ������̂ŁA�t�@�C����(�t���p�X)�������ɉ������B
      ����z�z��Plug-in�ł�filename�͎g���Ă��Ȃ��B


  �EGetPictureInfo - �摜�t�@�C���Ɋւ�����𓾂�
    Prototype:
      extern "C" int _export PASCAL GetPictureInfo(
         LPSTR buf,long len,unsigned int flag,struct PictureInfo *lpInfo);

    Parameter:
      LPSTR buf : ���͂��t�@�C���̏ꍇ �t�@�C����
                        �������[�̏ꍇ �t�@�C���C���[�W�ւ̃|�C���^
      long len  : ���͂��t�@�C���̏ꍇ �Ǎ��݊J�n�I�t�Z�b�g(MacBin�Ή��̂���)
                        �������[�̏ꍇ �f�[�^�T�C�Y
      unsigned int flag : �ǉ���� xxxx xxxx xxxx xSSS
                  SSS : ���͌`��
                      000 : �f�B�X�N�t�@�C��
                      001 : ��������̃C���[�W
      struct PictureInfo *lpInfo :
                  struct PictureInfo
                  {
                    long left,top;    �摜��W�J����ʒu
                    long width;       �摜�̕�(pixel)
                    long height;      �摜�̍���(pixel)
                    WORD x_density;   ��f�̐����������x
                    WORD y_density;   ��f�̐����������x
                    short colorDepth; �P��f�������bit��
                    HLOCAL hInfo;    �摜���̃e�L�X�g���
                  };
                  hInfo�ɂ͕K�v�ɉ�����Plug-in���m�ۂ���Global�������[��
                  �n���h�����i�[�����B

    Return:
      �G���[�R�[�h�B0�Ȃ琳��I���B

  �EGetPicture - �摜��W�J����
    Prototype:
      extern "C" int _export PASCAL GetPicture(
              LPSTR buf,long len,unsigned int flag,
              HANDLE *pHBInfo,HANDLE *pHBm,
              FARPROC lpPrgressCallback,long lData);

    Parameter:
      LPSTR buf : ���͂��t�@�C���̏ꍇ �t�@�C����
                        �������[�̏ꍇ �t�@�C���C���[�W�ւ̃|�C���^
      long len  : ���͂��t�@�C���̏ꍇ �Ǎ��݊J�n�I�t�Z�b�g(MacBin�Ή��̂���)
                        �������[�̏ꍇ �f�[�^�T�C�Y
      unsigned int flag : �ǉ���� xxxx xxxx xxxx xSSS
                  SSS : ���͌`��
                      000 : �f�B�X�N�t�@�C��
                      001 : ��������̃C���[�W
      HLOCAL *pHBInfo : BITMAPINFO �\���̂��[�߂�ꂽ�������n���h����
                             �Ԃ����B
      HLOCAL *pHBm    : �r�b�g�}�b�v�f�[�^�{�̂̃������n���h�����Ԃ����
      FARPROC lpPrgressCallback :
                �r���o�߂�\������R�[���o�b�N�֐��ւ̃|�C���^�B
                NULL�̏ꍇ�Aplug-in�͏������I������܂Ńv���Z�X���L���A
                ���f���o���܂���B
                �R�[���o�b�N�֐���prototype:
                  int PASCAL ProgressCallback(
                                        int nNum,int nDenom,long lData);
                  �܂� nNum==0 �ŃR�[������AnNum==nDenom �ɂȂ�܂�
                  ����I�ɌĂ΂��B
                  �ߒl�� ��0 �̎��APlug-in�͏����𒆒f����B
      long lData : �R�[���o�b�N�֐��ɓn��long�f�[�^�B
                  �|�C���^����K�v�ɉ����Ď󂯓n����B

    Return:
      �G���[�R�[�h�B0�Ȃ琳��I���B

    ���:
      �v���O�C����LocalAlloc�ɂ���ĕK�v�ȃ������[���m�ۂ��A���̃n���h����
      �Ԃ��B
      �A�v���P�[�V������LocalFree�ɂ���ă������[���J������K�v������B

  �EGetPreview - �v���r���[�E�J�^���O�\���p�摜�k���W�J���[�e�B��
    Prototype:
      extern "C" int _export PASCAL GetPreview(
              LPSTR buf,long len,unsigned int flag,
              HANDLE *pHBInfo,HANDLE *pHBm,
              FARPROC lpPrgressCallback,long lData);

    Parameter:
      GETPICTURE�Q�ƁB

    Return:
      �G���[�R�[�h�B0�Ȃ琳��I���B
      ���̊֐��̓I�v�V�����ł���A���Ή��̏ꍇ�� -1 ��Ԃ��B

    ���:
      �v���r���[���ŗp����k�����ꂽ�摜���t�@�C������쐬����B
      JPEG�̗l�ɁA�A���S���Y���̊֌W�ŏk�����ꂽ�T�C�Y�ł͍����ɓW�J�o����
      �Ƃ��ɂ��̊֐����C���v�������g����B
      ����z�z��Plug-in�ł� IFJPEG.PLG �̂ݑΉ�(1/4�T�C�Y�œW�J)���Ă���B
      ���Ή��̏ꍇ�ASusie�͒ʏ�̓W�J���[�e�B����p���ēW�J������
      �k���������s���B
      �i�Ή����Ă��Ă��k�����[�h���ꂽ�摜���X�ɃT�C�Y�������Ă���j
      �v���O�C����LocalAlloc�ɂ���ĕK�v�ȃ������[���m�ۂ��A���̃n���h����
      �Ԃ��B
      �A�v���P�[�V������LocalFree�ɂ���ă������[���J������K�v������B

  �E�G���[�R�[�h
    0 : ����I��
   -1 : ���̋@�\�̓C���v�������g����Ă��Ȃ�
    1 : �R�[���o�b�N�֐�����0��Ԃ����̂œW�J�𒆎~����
    2 : ���m�̃t�H�[�}�b�g
    3 : �f�[�^�����Ă���
    4 : �������[���m�ۏo���Ȃ�
    5 : �������[�G���[�iLock�o���Ȃ��A���j
    6 : �t�@�C�����[�h�G���[
    7 : �i�\��j
    8 : �����G���[

�T�D'00AM'�̊֐� (�b��)

  �EIsSupported - �W�J�\��(�Ή����Ă���)�t�@�C���`�������ׂ�B
    Prototype:
      extern "C" int _export PASCAL IsSupported(LPSTR filename,DWORD dw);

    Parameter:
      LPSTR filename : �t�@�C���l�[��
      DWORD dw       : ��ʃ��[�h��  0  �̂Ƃ�:
                           �t�@�C���n���h��
                       ��ʃ��[�h�� ��0 �̂Ƃ�:
                           �t�@�C���擪��(2Kbyte�ȏ�)��ǂݍ��񂾃o�b�t�@�ւ�
                           �|�C���^
                           �t�@�C���T�C�Y��2Kbyte�ȉ��̏ꍇ���o�b�t�@��2Kbyte
                           �m�ۂ��A�]���� 0 �Ŗ��߂邱��

    Return:
      �Ή����Ă���摜�t�H�[�}�b�g�ł���Δ�0��Ԃ�

    ���:
      �ڂ�����'00IN'��ISSUPPORTED�֐����Q�Ƃ̎��B
      ����dw�œn���o�b�t�@�T�C�Y2Kbyte�ȏ�͎��ȉ𓀌^LHa�Ή��̂��߁B

  �EGetArchiveInfo - �A�[�J�C�u���̂��ׂẴt�@�C���̏����擾����
    Prototype:
      extern "C" errcode _export PASCAL GetArchiveInfo(LPSTR buf,long len,
            unsigned int flag,HLOCAL *lphInf);

    Parameter:
      LPSTR buf : ���͂��t�@�C���̏ꍇ �t�@�C����
                        �������[�̏ꍇ �t�@�C���C���[�W�ւ̃|�C���^
      long len  : ���͂��t�@�C���̏ꍇ �Ǎ��݊J�n�I�t�Z�b�g(MacBin�Ή��̂���)
                        �������[�̏ꍇ �f�[�^�T�C�Y
      unsigned int flag : �ǉ���� xxxx xxxx xxxx xSSS
                  SSS : ���͌`��
                      000 : �f�B�X�N�t�@�C��
                      001 : ��������̃C���[�W
      HLOCAL *lphInf
                : �t�@�C�����̓������n���h�����󂯎��ϐ��ւ̃|�C���^�B
                  Plug-in���Ŋm�ۂ��ꂽLOCAL�������[�Ɏ��̍\���̔z��
                  �������܂�A���̃n���h�����Ԃ����B
                  method[0]=='\0'�ŏI�[�B
                        typedef struct
                        {
                            unsigned char method[8];    ���k�@�̎��
                            unsigned long position;     �t�@�C����ł̈ʒu
                            unsigned long compsize;     ���k���ꂽ�T�C�Y
                            unsigned long filesize;     ���̃t�@�C���T�C�Y
                            time_t timestamp;           �t�@�C���̍X�V����
                            char path[200];             ���΃p�X
                            char filename[200];         �t�@�C���l�[��
                            unsigned long crc;         CRC
                        } fileInfo;

    Return:
      �G���[�R�[�h�B0�Ȃ琳��I���B

  �EGetFileInfo - �A�[�J�C�u���̎w�肵���t�@�C���̏����擾����
    Prototype:
      extern "C" errcode _export PASCAL GetFileInfo(LPSTR buf,long len,
                    LPSTR filename, unsigned int flag,fileInfo *lpInfo);
    Parameter:
      LPSTR buf : ���͂��t�@�C���̏ꍇ �t�@�C����
                        �������[�̏ꍇ �t�@�C���C���[�W�ւ̃|�C���^
      long len  : ���͂��t�@�C���̏ꍇ �Ǎ��݊J�n�I�t�Z�b�g(MacBin�Ή��̂���)
                        �������[�̏ꍇ �f�[�^�T�C�Y
      LPSTR filename : �����擾����t�@�C���̃t�@�C���l�[��
                        �A�[�J�C�u���̑��΃p�X���܂߂Ďw��
      unsigned int flag : �ǉ���� xxxx xxxx Ixxx xSSS
                  SSS : ���͌`��
                      000 : �f�B�X�N�t�@�C��
                      001 : ��������̃C���[�W
                  I : 0 : �t�@�C�����̑啶������������ʂ���
                      1 : �t�@�C�����̑啶���������𓯈ꎋ����B
      fileInfo *lpInfo
                : �����󂯎��fileInfo�\���̂ւ̃|�C���^

    Return:
      �G���[�R�[�h�B0�Ȃ琳��I���B

  �EGetFile - �A�[�J�C�u���̃t�@�C�����擾����
    Prototype:
      extern "C" errcode _export PASCAL GetFile(LPSTR src,long len,
                LPSTR dest,unsigned int flag,
                FARPROC prgressCallback,long lData);

    Parameter:
      LPSTR src  : ���͂��t�@�C���̏ꍇ �t�@�C����
                        �������[�̏ꍇ �t�@�C���C���[�W�ւ̃|�C���^
      long len   : ���͂��t�@�C���̏ꍇ �Ǎ��݊J�n�I�t�Z�b�g
                        �������[�̏ꍇ �f�[�^�T�C�Y
      void far *dest : �o�͐悪�t�@�C���̏ꍇ
                              �o�͐�f�B���N�g��
                               (���ɓ��̑��΃p�X�͖��������)
                        �������[�̏ꍇ 
                              �t�@�C���̓�����LOCAL�������[�n���h�����󂯎��
                              �ϐ��ւ̃|�C���^�B
      unsigned int flag  : �ǉ���� xxxx xDDD xxxx xSSS
                  SSS : ���͌`��
                      000 : �f�B�X�N�t�@�C��
                      001 : ��������̃C���[�W
                  DDD : �o�͌`��
                      000 : �f�B�X�N�t�@�C��
                      001 : ��������̃C���[�W
      FARPROC lpPrgressCallback :
                �r���o�߂�\������R�[���o�b�N�֐��ւ̃|�C���^�B
                NULL�̏ꍇ�Aplug-in�͏������I������܂Ńv���Z�X���L���A
                ���f���o���܂���B
                �R�[���o�b�N�֐���prototype:
                  int PASCAL ProgressCallback(
                                        int nNum,int nDenom,long lData);
                  �܂� nNum==0 �ŃR�[������AnNum==nDenom �ɂȂ�܂�
                  ����I�ɌĂ΂��B
                  �ߒl�� ��0 �̎��APlug-in�͏����𒆒f����B
      long lData : �R�[���o�b�N�֐��ɓn��long�f�[�^�B
                  �|�C���^����K�v�ɉ����Ď󂯓n����B

    Return:
      �G���[�R�[�h�B0�Ȃ琳��I���B

    ���:
      �v���O�C����LocalAlloc�ɂ���ĕK�v�ȃ������[���m�ۂ��A���̃n���h����
      �Ԃ��B
      �A�v���P�[�V������LocalFree�ɂ���ă������[���J������K�v������B

�U�DPlug-in�̎g����
  Plug-in��DLL�ł�����A�ʏ��DLL�Ɠ����p�Ɏ��̂Q�̕��@�ŃA�v���P�[�V������
  �����N�o���܂��B

  1) DLL����C���|�[�g���C�u����������ă����N����
    implib.exe �� implibw.exe ���g����Plug-in����C���|�[�g���C�u������
    ����āA������A�v���P�[�V�����Ƀ����N���܂��B
    ���̕��@�͊ȒP�ł����A�����Plug-in�����g���܂���B
  2) LoadLibrary �ŕK�v�ɉ����ă����N����B
    ���̕��@�͏��X��Ԃ�������܂����A�������Č�������Plug-in�𓮓I��
    �p���邱�Ƃ��ł��܂��B

  �ʏ��1)�̕��@���p�����܂����A�����̃t�H�[�}�b�g�ɑΉ�����K�v������
  �ꍇ�ɂ�2)�̕��@���������߂��܂��B
  2)�̕��@��p������̂Ƃ��đS�̗̂����������܂��B

  1.Plug-in����������B
    Plug-in�̂���f�B���N�g����"*.plg"�Ō������A�����������̂�
    LoadLibrary �Ń��[�h���܂��B
    GetProcAddress �� GETPLUGININFO �֐��ւ̃|�C���^���擾���A
    GETPLUGININFO �֐��ɂď��ԍ�0��Plug-in�o�[�W�������m���߂܂��B
    �Ή����Ă���o�[�W�����Ȃ�Plug-in���X�g�ɉ����܂��B
    �Ή����Ă��Ȃ����̂Ȃ�FreeLibrary�ŖY�ꂸ�ɊJ�����܂��B

  2.�摜�t�@�C���ɍ�����Plug-in��T���B
    �摜�t�@�C�������[�h����K�v���������Ȃ�܂����̃t�@�C����_lopen����
    �I�[�v�����܂��B
    ���� Plug-in���X�g�ɂ��������ď��� ISSUPPORTED �֐����ĂсA�Ή�����
    Plug-in��T���܂��BMacBinary ���t���Ă���\��������̂ŁAoffset=0��
    ���߂ȏꍇ�� offset=128 �ł�����x�T���Ɨǂ��ł��傤�B

  3.�摜��W�J����B
    �Ή�����Plug-in�����������炻��Plug-in�� GETPICTURE �֐��Ń��[�h���܂��B
    CALLBACK�֐����� PeekMessage ���g�����ő��̃v���Z�X��(�����Ď����ɂ�)
    ���s�̋@���^����ƃX�}�[�g�ł��B

  4.Plug-in���J������B
    �A�v���P�[�V�������I�����鎞�ɂ͖Y�ꂸ�� LoadLibrary ����Plug-in���ׂĂ�
    FreeLibrary �ŊJ�����܂��傤�B


�V�DPlug-in�̎d�l�Ǝg�p�Ɋւ���
  Plug-in����肽���A�������͎g������������ł͂悭�킩���A�Ƃ�������
  ���LID�܂ł��₢���킹�������B�Ȃɂ����珕���o����Ǝv���܂��B�i�Ԏ���
  �x���Ȃ��Ă��{��Ȃ��ł�(^_^;)�j
  �܂��AAPI�̎d�l�Ɋւ��Ă̌�ӌ������҂����Ă���܂��BAPI�o�[�W�����A�b�v��
  �ɎQ�l�ɂ����Ă��������܂��B���̃o�[�W������ Susie �̓����N���X��I/F��
  �قƂ�ǂ��̂܂܂Ȃ̂Ŕėp���Ɍ����܂���(^_^;)
  �]�ړ��Ɋւ��Ă� plugin.txt ���Q�Ƃ��ĉ������B

	Nifty-serve GGB01506   �|���Ðl (��������)
*/