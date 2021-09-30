clc
clear

%% Attention: To be modified by user!
FLOAT_VERSION       = 0; % 0 --> Fixed-point; 1 --> Float-point
ENABLE_1BEAM_OUTPUT = 0; % 0 --> Generate 3-beam WB; 1 --> Generate 1-beam WB 

WB_name = 'WB_2mic_160mm_1203'; % Select the name of WB mat file

%% Squeeze B matrix if necessary.
load(['./', WB_name, '.mat']);      

disp('size of W:')
size(W)
disp('size of B:')
size(B)

if ndims(B) == 4
    B(:, 2, :, :) = [];
    B = squeeze(B); % Convert to 2x257x3 matrix
end
size(B)

%% Generate 3-Beam WB.
if ENABLE_1BEAM_OUTPUT == 0
    if FLOAT_VERSION == 1
        fname = fopen(['LUDA_FLOAT_', WB_name, '_3EntBeam.h'], 'w');
    else
        fname = fopen(['LUDA_FIXED_', WB_name, '_3EntBeam.h'], 'w');
    end

    fprintf(fname, '#ifndef __LUDA_WB_H__\n');
    fprintf(fname, '#define __LUDA_WB_H__\n\n');

    if FLOAT_VERSION == 1
        fprintf(fname, 'const static GSC_S32_T g_asGscWq[] = {\n');
    else
        fprintf(fname, '/* W32Q24 */\nconst static GSC_S32_T g_asGscWq[] = {\n');
    end
    % W is interpreted as W32Q24
    for beam = 1:3
        for mics = 1:2
            fprintf(fname, '    /* mic%d beam%d */\n', mics, beam);
            for bins = 1:257
    if FLOAT_VERSION == 1
                fprintf(fname, '    %.10f,',   real(W(mics, bins, beam)));
                fprintf(fname, '    %.10f,\n', imag(W(mics, bins, beam)));
    else
                fprintf(fname, '    %d,',   round(real(W(mics, bins, beam)*2^24)));
                fprintf(fname, '    %d,\n', round(imag(W(mics, bins, beam)*2^24)));
    end
            end
        end
    end
    fprintf(fname, '};\n\n');

    if FLOAT_VERSION == 1
        fprintf(fname, 'const static GSC_S32_T g_asGscB[]  = {\n');
    else
        fprintf(fname, '/* W32Q24 */\nconst static GSC_S32_T g_asGscB[]  = {\n');
    end
    % B is interpreted as W32Q24
    for beam = 1:3
        for mics = 1:2
            fprintf(fname, '    /* mic%d beam%d */\n', mics, beam);
            for bins = 1:257
    if FLOAT_VERSION == 1
                fprintf(fname, '    %.10f,',   real(B(mics, bins, beam)));
                fprintf(fname, '    %.10f,\n', imag(B(mics, bins, beam)));
    else
                fprintf(fname, '    %d,',   round(real(B(mics, bins, beam)*2^24)));
                fprintf(fname, '    %d,\n', round(imag(B(mics, bins, beam)*2^24)));
    end
            end
        end
    end
    fprintf(fname, '};\n\n');

    fprintf(fname, '#endif\n');
    fclose all;

%% Generate 1-Beam WB.
elseif ENABLE_1BEAM_OUTPUT == 1
    if (isequal(size(W), [2 257 3]) && (isequal(size(B), [2 257 3])))
        disp('Select 1-Beam WB from 3-Beam WB.');
        W = W(:, :, 2);
        B = B(:, :, 2);
    elseif (isequal(size(W), [2 257 1]) && (isequal(size(B), [2 257 1])))
        disp('Only 1-Beam WB avaiable.');
    else
        disp('WB is error.');
    end
    
    
    if FLOAT_VERSION == 1
        fname = fopen(['LUDA_FLOAT_', WB_name, '_1EntBeam.h'], 'w');
    else
        fname = fopen(['LUDA_FIXED_', WB_name, '_1EntBeam.h'], 'w');
    end

    fprintf(fname, '#ifndef __LUDA_WB_H__\n');
    fprintf(fname, '#define __LUDA_WB_H__\n\n');

    if FLOAT_VERSION == 1
        fprintf(fname, 'const static GSC_S32_T g_asGscWq[] = {\n');
    else
        fprintf(fname, '/* W32Q24 */\nconst static GSC_S32_T g_asGscWq[] = {\n');
    end
    % W is interpreted as W32Q24
    for beam = 1
        for mics = 1:2
            fprintf(fname, '    /* mic%d beam%d */\n', mics, beam);
            for bins = 1:257
    if FLOAT_VERSION == 1
                fprintf(fname, '    %.10f,',   real(W(mics, bins)));
                fprintf(fname, '    %.10f,\n', imag(W(mics, bins)));
    else
                fprintf(fname, '    %d,',   round(real(W(mics, bins)*2^24)));
                fprintf(fname, '    %d,\n', round(imag(W(mics, bins)*2^24)));
    end
            end
        end
    end
    fprintf(fname, '};\n\n');

    if FLOAT_VERSION == 1
        fprintf(fname, 'const static GSC_S32_T g_asGscB[]  = {\n');
    else
        fprintf(fname, '/* W32Q24 */\nconst static GSC_S32_T g_asGscB[]  = {\n');
    end
    % B is interpreted as W32Q24
    for beam = 1
        for mics = 1:2
            fprintf(fname, '    /* mic%d beam%d */\n', mics, beam);
            for bins = 1:257
    if FLOAT_VERSION == 1
                fprintf(fname, '    %.10f,',   real(B(mics, bins)));
                fprintf(fname, '    %.10f,\n', imag(B(mics, bins)));
    else
                fprintf(fname, '    %d,',   round(real(B(mics, bins)*2^24)));
                fprintf(fname, '    %d,\n', round(imag(B(mics, bins)*2^24)));
    end
            end
        end
    end
    fprintf(fname, '};\n\n');

    fprintf(fname, '#endif\n');
    fclose all; 

%% Parameter Error
else
    disp('Parameter ENABLE_1BEAM_OUTPUT Error.');
end

