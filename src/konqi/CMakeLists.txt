
tde_create_translated_desktop(
  SOURCE
    k3b_create_data_cd.desktop k3b_create_data_dvd.desktop
    k3b_create_audio_cd.desktop k3b_create_video_cd.desktop
    k3b_write_bin_image.desktop k3b_write_iso_image.desktop
  DESTINATION ${DATA_INSTALL_DIR}/k3b/servicemenus
)

tde_create_translated_desktop(
  SOURCE
    k3b_audiocd_rip.desktop k3b_videodvd_rip.desktop
    k3b_cd_copy.desktop k3b_dvd_copy.desktop
    k3b_handle_empty_dvd.desktop k3b_handle_empty_cd.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus
)
