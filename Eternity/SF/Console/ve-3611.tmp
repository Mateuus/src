// prefix
// d_ - debug
// r_ - render
// g_ - game
// ph_ - physics
// ch_ - cheats


REG_VAR( d_mouse_window_lock,		false,			0 );		// should I restrict mouse at window client area?
REG_VAR( d_show_scene_luma,			0,				0 );
#ifndef FINAL_BUILD
REG_VAR( d_show_muzzle,				0,				0 );
REG_VAR( d_map_force_load,			"",				0 );		// map 
REG_VAR( d_ps3_bin_dir,				"",				0 );		// ps3 bin dir 
REG_VAR( d_gfxlog_filter,			true,			0 );		// scaleform log messages filter
REG_VAR( d_video_spectator_mode,	false,			0 );		// spectator mode in game
REG_VAR( d_disable_game_hud,		false,			0 );		// disable HUD rendering
REG_VAR( d_disable_render_hud,		false,			0 );
REG_VAR( d_force_scope_rendering,	false,			0 );		// if you want to disable HUD but leave scope on
REG_VAR( d_spectator_slow_move_mul, 0.25f,			0 );		// for spectator mode, when pressing Ctrl it will slow down that amount
REG_VAR( d_spectator_fast_move_mul, 4.0f,			0 );		// same, but when press Shift
REG_VAR( d_observer_mode,			false,			0 );		// special observer mode
REG_VAR( d_spectator_fov,			0.0f,			0 );
REG_VAR( d_profile_dump_num_frames, 10,				0 );
REG_VAR( d_do_ragdoll,				false,			0 );
REG_VAR( d_show_checker_texture,	0,				0 );
REG_VAR( d_visualize_tex_density,	0,				0 );

REG_VAR( d_temp_green_screen,		false,			0 );

#if VEHICLES_ENABLED
REG_VAR( d_drive_vehicles,			false,			0 );		
#endif // VEHICLES_ENABLED
#endif // FINAL BUILD

#if ENABLE_WEB_BROWSER
REG_VAR( d_url_navigate,			"",				0 );
REG_VAR( d_show_browser,			false,			0 );
#endif

REG_VAR( d_job_chief_idle_events,	true,			0 );

REG_VAR( r_bpp,						32,				0 );		// color bpp

REG_VAR( r_near_plane,				1.f,			0 );
REG_VAR( r_far_plane,				32000.f,		0 );

REG_VAR_C( r_ssao					, 1, 0, 1,		VF_CONSTRAINT );		// ssao on/off
REG_VAR_C( r_ssao_blur_w_normals	, 1, 0,	1,		VF_CONSTRAINT );
REG_VAR_C( r_double_depth_ssao		, 0, 0, 1,		VF_CONSTRAINT );

#ifndef FINAL_BUILD
REG_VAR( r_show_reflectivity,		false,			0 );
#endif

REG_VAR( r_roads_zdisplace,			1.f,			0 );
REG_VAR( r_grass_anim_speed,		3.f,			0 );
REG_VAR( r_grass_anim_amp,			0.04f,			0 );

#define R3D_MAX_GRASS_PATCH_DISTANCE_COEF 2.0f

REG_VAR( r_grass_view_dist,			64.f,			0 );
REG_VAR( r_grass_view_coef,			1.f,			0 );
REG_VAR( r_grass_show_debug,		false,			0 );
REG_VAR( r_grass_draw,				true,			0 );
REG_VAR( r_grass_skip_step,			0,				0 );

#ifndef FINAL_BUILD
REG_VAR( r_grass_show_boxes,		0,				0 );
REG_VAR( r_show_shadow_extrusions,	false,			0 );
REG_VAR( r_show_scenebox,			false,			0 );
REG_VAR( r_show_bbox,				false,			0 );
#endif

REG_VAR( r_terrain,					true,			0 );

REG_VAR( r_do_dyn_shadow_blur,		true,			0 );
REG_VAR( r_shadows_blur_phys_range,	512.f,			0 );
REG_VAR( r_shadows_blur_bias,		0.22f,			0 );
REG_VAR( r_shadows_blur_sense,		32.f,			0 );
REG_VAR( r_shadows_blur_radius,		1.25f,			0 );
REG_VAR( r_active_shadow_slices,	3,				0 );


REG_VAR( r_relief_decals,			false,			0 );
REG_VAR( r_shadowcull,				true,			0 );
REG_VAR( r_shadowcull_extrude,		256.f,			0 );
REG_VAR( r_ssao_stencil_cut,		true,			0 );

REG_VAR( r_shadow_extrusion_limit,	100.0f,			0 );

REG_VAR( r_shadows,					true,			0 );
REG_VAR( r_particle_shadows,		true,			0 );
REG_VAR( r_cw_shadows,				false,			0 );

REG_VAR( r_decal_normals,			false,			0 );
#ifndef FINAL_BUILD
REG_VAR( r_terra_wire,				false,			0 );
#endif

REG_VAR( r_gameplay_blur_strength,	0,				0 );
REG_VAR( r_force_aspect,			0,				0 ); // 0 - no forcing, otherwise set desired aspect.
REG_VAR( r_screen_scale_x,			1,				0 );
REG_VAR( r_screen_scale_y,			1,				0 );
REG_VAR( r_screen_offset_x,			0,				0 );
REG_VAR( r_screen_offset_y,			0,				0 );

REG_VAR( r_occ_bbox_expand_coef,	0.04f,			0 );
REG_VAR( r_occ_bbox_add_coef,		0.05f,			0 );
REG_VAR( r_warmup,					0,				0 );
REG_VAR( r_allow_warmup,			1,				0 );

REG_VAR( r_terminateOnZ,			0,				0 );
REG_VAR( r_render_on_deactivation,	0,				0 );
REG_VAR( r_ini_read,				0,				0 );

#if 0
REG_VAR( r_smooth_normals,			false,			0 );
REG_VAR( r_smooth_normals_thresh,	0.05f,			0 );
REG_VAR( r_smooth_normals_maxd,		0.33f,			0 );
REG_VAR( r_smooth_normals_amplify,	96.0f,			0 );
REG_VAR( r_smooth_normals_taps,		4,				0 );
#endif

REG_VAR( r_simplify_pure_shadows	, true		, 0 );

#ifndef FINAL_BUILD
REG_VAR( r_debug_helper				, 0			, 0 );
REG_VAR( r_show_light_helper		, 0			, 0 );
REG_VAR( r_hide_icons				, 0			, 0 );
REG_VAR( r_hide_editor_statusbar	, 0			, 0 );

REG_VAR( r_show_budgeter			, false		, 0 );
REG_VAR( r_show_profiler			, false		, 0 );
REG_VAR( r_show_d3dmarks			, false		, 0 );
REG_VAR( r_profiler_scaleform		, false		, 0 );
REG_VAR( r_profiler_d3d				, false		, 0 );
REG_VAR( r_profiler_paused			, true		, 0 );
REG_VAR( r_profiler_hierarchy		, true		, 0 );
REG_VAR( r_show_player_debug_data	, false		, 0 );
REG_VAR( r_show_player_health		, false		, 0 );
#endif

REG_VAR( r_instanced_particles		, true		, 0 );
REG_VAR( r_multithreading			, true		, 0 );
REG_VAR( r_use_oq					, true		, 0 );
REG_VAR( r_use_shared_animtracks	, true		, 0);

// Filmic Tone operator params
REG_VAR( r_film_tone_a				, 0.15f		, 0 );
REG_VAR( r_film_tone_b				, 0.50f		, 0 );
REG_VAR( r_film_tone_c				, 0.10f		, 0 );
REG_VAR( r_film_tone_d				, 0.20f		, 0 );
REG_VAR( r_film_tone_e				, 0.02f		, 0 );
REG_VAR( r_film_tone_f				, 0.30f		, 0 );
REG_VAR( r_white_level				, 11.2f		, 0 );

// Scene exposure Bias + params for light adaptation
REG_VAR( r_exposure_bias			, 0.0f		, 0 );
REG_VAR( r_exposure_minl			, 0.0f		, 0 );
REG_VAR( r_exposure_maxl			, 0.5f		, 0 );
REG_VAR( r_exposure_targetl			, 0.2f		, 0 );
REG_VAR( r_exposure_rangeMax		, 0.0f		, 0 );
REG_VAR( r_exposure_rangeMin		, 0.0f		, 0 );
REG_VAR( r_need_reset_exporue		, 0			, 0 );
REG_VAR( r_last_exposure_source		, 0			, 0 );
REG_VAR( r_light_adapt_speed_pos	, 1.0f		, 0 );
REG_VAR( r_light_adapt_speed_neg	, 1.0f		, 0 );

REG_VAR( r_gameui_exposure			, 1.0f		, 0 ); 

#ifndef FINAL_BUILD
REG_VAR( r_draw_particles			, 1			, 0 );
REG_VAR( r_draw_meshes				, 1			, 0 );
REG_VAR( r_disable_mesh_textures	, 0			, 0 );
#endif

REG_VAR( r_transp_shadows			, 0			, 0 );
REG_VAR( r_force_parts_transp_shad	, 0			, 0 );
REG_VAR( r_transp_shadow_dist		, 100.f		, 0 );
REG_VAR( r_transp_shadowmap_size	, 512		, 0 );
REG_VAR( r_transp_shadowmap_fade	, 32.f		, 0 );
REG_VAR( r_transp_shadow_coef		, 0.9f		, 0 );

REG_VAR( r_occlusion_vis_helper		, true		, 0 )

REG_VAR( r_half_depth_particles		, 1			, 0 ); // turned off by default
REG_VAR( r_half_depth_par_strerect	, 0			, 0 );
REG_VAR( r_half_res_particles		, 1			, 0 );
REG_VAR( r_avarage_fps				, 1			, 0 );
REG_VAR( r_limit_fps				, 0			, 0 );
REG_VAR( r_sleep_on_limit_fps		, 0			, 0 );
REG_VAR( r_show_terra_order			, 0			, 0 );
REG_VAR( r_show_draw_order			, 0			, 0 ); // 1 - meshes, 2 - particles

REG_VAR( r_split_grass_render		, 0			, 0 );
REG_VAR( r_allow_delayed_queries	, 1			, 0 );

REG_VAR( r_precalc_shadowcull		, 1			, 0 );
REG_VAR( r_inst_precalc_shadowcull	, 1			, 0 );

#ifndef FINAL_BUILD
REG_VAR( r_show_shadow_scheme		, 0			, 0 );
#endif

REG_VAR( r_optimize_shadow_map		, 1			, 0 );

REG_VAR( r_sss		, true	, 0 );

REG_VAR( r_show_grass_tint			, 0			, 0 );
REG_VAR( r_show_grass_tint_scale	, 1.f		, 0 );

REG_VAR( r_decals					, 1			, 0 );
REG_VAR( r_decals_proximity_multiplier	, 1.0f	, 0 );

REG_VAR( r_lfsm_cache_dist			, 8.f		, 0 );
REG_VAR( r_lfsm_wrap_mode			, 1			, 0 );

REG_VAR( r_shadow_low_size_coef		, 0.5f		, 0 );
REG_VAR( r_sky_intensity			, 1.0f		, 0 );

REG_VAR( r_depth_mode				, 0			, 0 );

// gamepad camera view
REG_VAR( r_gamepad_view_sens,		40.0f			, 0	);
REG_VAR( r_gamepad_move_speed,		5.f			, 0	);
#ifndef FINAL_BUILD
REG_VAR( r_video_fov,				60.0f		, 0);
REG_VAR( r_video_nearDOF_start,		1.0f		, 0);
REG_VAR( r_video_nearDOF_end,		2.0f		, 0);
REG_VAR( r_video_farDOF_start,		300.0f		, 0);
REG_VAR( r_video_farDOF_end,		500.0f		, 0);
#endif

//------------------------------------------------------------------------
// Vars modified through Settings Menu
REG_VAR( g_num_game_executed2    , 0			, VF_SAVE ); // number of times game was executed
REG_VAR( g_num_matches_played	 , 0			, VF_SAVE );
REG_VAR( g_user_save_login		, false		, VF_SAVE );
REG_VAR( g_user_login_info		, ""		, VF_SAVE );
REG_VAR( g_vertical_look		, false		, VF_SAVE );
REG_VAR( g_mouse_wheel			, true		, VF_SAVE );
REG_VAR( g_mouse_acceleration	, false		, VF_SAVE );
//REG_VAR( g_hint_system			, true		, VF_SAVE );
REG_VAR( g_enable_voice_commands, true		, VF_SAVE );
REG_VAR( g_toggle_aim			, false		, VF_SAVE );
REG_VAR( g_toggle_crouch		, false		, VF_SAVE );
REG_VAR( g_user_language		, ""		, VF_SAVE );
REG_VAR( g_tps_camera_mode		, 0,		VF_SAVE); // temp var to save TPS camera mod settings
REG_VAR_C( g_camera_mode		, 1,    0, 2, VF_SAVE|VF_CONSTRAINT ); // 0 - TPS mode, crosshair in center, 1- TPS mode, crosshair off-center, 2 - FPS mode

REG_VAR_C( r_server_region,	-1,			-1,	2, VF_SAVE | VF_CONSTRAINT); // 0 - us west, 1-eu, 2 - any

REG_VAR_C( g_mouse_sensitivity		, 1.f		, 0.1f, 10.0f, VF_SAVE | VF_CONSTRAINT);
REG_VAR_C( s_sound_volume			, 1.f		, 0.0f, 1.0f, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( s_music_volume			, 1.f		, 0.0f, 1.0f, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( s_comm_volume			, 1.f		, 0.0f, 1.0f, VF_SAVE | VF_CONSTRAINT );

REG_VAR_C( r_brightness				, 0.5f		, 0.125f, 0.875f, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_contrast				, 0.5f		, 0.125f, 0.875f, VF_SAVE | VF_CONSTRAINT );

REG_VAR( r_width				, 1024		, VF_SAVE );		// width window
REG_VAR( r_height				, 768		, VF_SAVE );		// height window

REG_VAR_C( r_overall_quality		, 2			, 1, 5, VF_SAVE | VF_CONSTRAINT );
REG_VAR( r_fullscreen			, true			, VF_SAVE );		// fullscreen mode
REG_VAR( r_fullscreen_load		, true			, 0 );		// this value will hold r_fullscreen after loading ini file
REG_VAR( r_apex_enabled			, false			, VF_SAVE );

REG_VAR_C( r_mesh_quality			, 3			, 1, 3, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_texture_quality		, 2			, 1, 3, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_terrain_quality		, 2			, 1, 3, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_decoration_quality		, 2			, 1, 3, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_water_quality			, 2			, 1, 3, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_shadows_quality		, 2			, 1, 4, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_lighting_quality		, 2			, 1, 3, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_particles_quality		, 2			, 1, 4, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_antialiasing_quality	, 2			, 1, 4, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_anisotropy_quality		, 2			, 1, 4, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_postprocess_quality	, 2			, 1, 3, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_ssao_quality			, 2			, 1, 5, VF_SAVE | VF_CONSTRAINT );
REG_VAR_C( r_vsync_enabled			, 0			, 0, 1,	VF_SAVE | VF_CONSTRAINT );

REG_VAR( r_driver_hash				, 0			, VF_SAVE );		//video driver update check
REG_VAR( r_out_of_vmem_encountered	, 0			, VF_SAVE );
//------------------------------------------------------------------------
// Vars, derived from the vars which are influenced by settings menu

REG_VAR( r_max_texture_dim		, 0			, 0 );

REG_VAR_C( r_half_scale_ssao		, 0		, 0	, 1, VF_CONSTRAINT );
REG_VAR_C( r_ssao_method			, 1		, 1 , 2, VF_CONSTRAINT );

REG_VAR( r_max_mesh_lod,			0,				0 );
REG_VAR( r_shared_sm_size,			512,			0 );
REG_VAR( r_shared_sm_cube_size,		512,			0 );
REG_VAR( r_dir_sm_size,				1024,			0 );
REG_VAR( r_shadow_blur,				true,			0 );
REG_VAR( r_shadow_light_source_render_dist, 50.0f,			0 );

/**	
	r_hardware_shadow_method:
	0 - R32F shadow
	1 - hardware PCF shadows
	2 - INTZ shadows
	3 - INTZ + R32F copy shadows
*/
#define HW_SHADOW_METHOD_R32F			0
#define HW_SHADOW_METHOD_HW_PCF			1
#define HW_SHADOW_METHOD_INTZ			2

REG_VAR( r_hardware_shadow_method,	0,			0 );
REG_VAR( r_hw_shadows_bias_amplify,	5.f,		0 );
REG_VAR( r_allow_hw_shadows,		1,			0 );

REG_VAR( r_terra_shadows,			true,			0 );
REG_VAR( r_anisotropy,				1,				0 );
REG_VAR( r_mlaa,					0,				0 );
REG_VAR( r_mlaa_pow2_len,			0,				0 );
REG_VAR( r_fxaa,					0,				0 );
REG_VAR( r_fxaa_shadow_blur,		0,				0 );

REG_VAR( r_dof,						0,				0 );
REG_VAR( r_film_grain,				0,				0 );
REG_VAR( r_bloom,					0,				0 );
REG_VAR( r_glow,					0,				0 );
REG_VAR( r_sun_rays,				0,				0 );

REG_VAR( r_gamma_pow,				1.f,			0 );
REG_VAR( r_allow_gpu_timestamps,	1,				0 );

REG_VAR( r_3d_stereo_post_fx,		0,				0 );
REG_VAR( r_3d_stereo_separation,	0.08f,			0 );
REG_VAR( r_3d_stereo_convergance,	0.33f,			0 );

REG_VAR( r_hud_filter_mode,			0,				0 );

REG_VAR( r_depth_screenshot_start,	0.f,			0 );
REG_VAR( r_depth_screenshot_end,	4096.f,			0 );
REG_VAR( r_depth_screenshot_name,	"depth.tga",	0 );
REG_VAR( r_do_depth_screenshot,		0,				0 );
REG_VAR( r_in_minimap_render,		0,				0 );
REG_VAR( r_allow_depth_screenshot,	0,				0 );
REG_VAR( r_do_screenshot,			0,				0 );
REG_VAR( r_do_internal_screenshot,	0,				0 );
REG_VAR( r_screenshot_name,			"screen.tga",	0 );

REG_VAR( r_optimized_ssao,			0,				0 );
REG_VAR( r_distort,					1,				0 );

REG_VAR( r_aura_extrude,			0.033f,			0 );
REG_VAR( r_score_sort,				0,				0 );
REG_VAR( r_rend_draw_limit,			0x7fffffff,		0 ); // for debugging, works in editor only
REG_VAR( r_limit_renderables,		0,				0 ); // for debugging, works in editor only
REG_VAR( r_last_def_rend_count,		0,				0 );

//------------------------------------------------------------------------
#ifndef FINAL_BUILD
REG_VAR( cl_miniconsole,			0,				0 );		// 
REG_VAR( cl_miniconsoletime,		0,				0 );		// 

REG_VAR( e_undo_depth,				64,				0 );		// 
REG_VAR( e_reposition_obj_5d,		false,			0 );		// 
REG_VAR( e_auto_save,				0,				0 );
REG_VAR( e_auto_save_folder,		"",				0 );
REG_VAR( e_auto_save_depth,			32,				0 );
REG_VAR( e_auto_save_interval,		5,				0 );
#endif

REG_VAR( g_trees,					true,			0 );		//	enable/disable trees
REG_VAR( r_trees_noninst_render,	false,			0 );

REG_VAR( g_serverip,			"127.0.0.1",	0 );
REG_VAR( g_api_ip,				"localhost", 0);
REG_VAR( g_locl_settings,			true,			0 );
REG_VAR( g_leveldata_xml_ver,		0,				0 );
REG_VAR( g_cursor_mode,				0,				VF_HIDDEN );
REG_VAR( g_hide_minimap,			0,				0 );
REG_VAR( g_force_rus_lang,			0,				0 );
REG_VAR( g_test_rus_client,			false,			0 );

REG_VAR( g_level_settings_ver,		0,				0 );

REG_VAR( g_async_d3dqueue,			0,				0 );
REG_VAR( r_local_vmem_size,			0,				0 );
REG_VAR( r_local_vmem_ddraw,		0,				0 );
REG_VAR( r_local_vmem_wmi,			0,				0 );

#ifndef FINAL_BUILD
REG_VAR( r_capture_vis_grid,		0,				0 );
REG_VAR( r_need_calc_vis_grid,		0,				0 );
REG_VAR( r_show_vis_grid,			0,				0 );

REG_VAR( r_debug_vis_grid_cell,		0,				0 );

REG_VAR( r_debug_vis_grid_cell_x,	0,				0 );
REG_VAR( r_debug_vis_grid_cell_y,	0,				0 );
REG_VAR( r_debug_vis_grid_cell_z,	0,				0 );

REG_VAR( r_vgrid_calc_one_cell,		0,				0 );
#endif

REG_VAR( r_target_shader_model,		0,				0 );
REG_VAR( r_card_shader_model,		0,				0 );

#ifndef FINAL_BUILD
REG_VAR( r_null_viewport,			0,				0 );
REG_VAR( r_full_zreject,			0,				0 );
REG_VAR( r_cut_meshes,				0,				0 );
REG_VAR( r_track_file_changes,		1,				0 );
REG_VAR( r_highlight_casters,		0,				0 );
REG_VAR( r_highlight_prepass,		0,				0 );

/**
 * r_scene_visualize_mode:
 * 0 - Standard scene rendering
 * 1 - White albedo texture (lighting visualization)
 * 2 - Diffuse scene buffer
 * 3 - Normal scene buffer
 * 4 - Aux scene buffer
 * 5 - Depth scene buffer
 */
#define SCENE_VISUALIZE_STANDARD	0
#define SCENE_VISUALIZE_LIGHTING	1
#define SCENE_VISUALIZE_AMBIENT		2
#define SCENE_VISUALIZE_DIFFUSE		3
#define SCENE_VISUALIZE_NORMALS		4
#define SCENE_VISUALIZE_AUX			5
#define SCENE_VISUALIZE_DEPTH		6

REG_VAR( r_scene_visualize_mode,	0,				0 );
#endif

REG_VAR( r_vmem_msg_target,			0,				0 );
REG_VAR( r_vmem_msg_shown,			0,				0 );

REG_VAR( r_scenebox_visgrid,		0,				0 );
REG_VAR( g_profile_time_delay,		3.0f,			0 );
REG_VAR( g_profile_frame_delay,		11,				0 );

REG_VAR( r_optimize_meshes,			0,				0 );
REG_VAR( r_device_clear,			1,				0 );
REG_VAR( r_need_gen_envmap,			0,				0 );

#define Z_PREPASS_METHOD_NONE 0
#define Z_PREPASS_METHOD_DIST 1
#define Z_PREPASS_METHOD_AREA 2

REG_VAR( r_z_allow_prepass,			1,						0 );
REG_VAR( r_z_prepass_method,		Z_PREPASS_METHOD_AREA,	0 );
REG_VAR( r_z_prepass_dist,			60.f,					0 );
REG_VAR( r_z_prepass_area,			0.11f,					0 );

REG_VAR( r_smart_transp_distort,	1,						0 );

#ifndef FINAL_BUILD
REG_VAR( r_force_scope,				0,						0 );
REG_VAR( r_show_collection_grid,	0,						0 );
#endif

REG_VAR( g_texture_edit_cmd,		"photoshop",			0 );
REG_VAR( g_async_loading,			false,					0 );
REG_VAR( d_random_editor_players,	false,					0 );
REG_VAR( d_spawn_players,			0,						0 );
REG_VAR( d_spawn_player_delay,		8,						0 );
REG_VAR( r_level_detail_radius,		100.0f,					0 );
REG_VAR( r_first_person_z_end,		0.01f,					0 );
REG_VAR( r_first_person,			true,					0 );
REG_VAR( r_first_person_render_z_start, 0.1f,				0 );
REG_VAR( r_first_person_render_z_end, 1.0f,					0 );
REG_VAR( r_first_person_fov,		50.0f,					0 ); // temp

REG_VAR( r_3d_vision_direct,		0,						0 );
REG_VAR( r_3d_stereo_swap_eyes,		0,						0 );
REG_VAR( r_3d_stereo_emu,			0,						0 );

REG_VAR( r_terrain2,				false,					0 );
REG_VAR( r_terrain2_anisotropy,		16,						0 );
REG_VAR( r_terrain2_padding,		4,						0 );

REG_VAR( g_char_spect,				0,						0 );
REG_VAR( g_char_spect_x,			0,						0 );
REG_VAR( g_char_spect_y,			0,						0 );
REG_VAR( g_char_spect_z,			0,						0 );
REG_VAR( g_char_spect_radius,		2,						0 );
REG_VAR( g_char_spect_angle_y,		0,						0 );
REG_VAR( g_char_spect_angle_x,		45,						0 );
REG_VAR( g_char_spect_angle_vel,	90,						0 );

#ifndef FINAL_BUILD
REG_VAR( r_terrain2_show_atlas,			0,					0 );
REG_VAR( r_terrain2_show_atlas_volume,	0,					0 );
REG_VAR( r_terrain2_show_atlas_scale,	1.0f,				0 );
REG_VAR( r_terrain2_show_atlas_off_x,	0.0f,				0 );
REG_VAR( r_terrain2_show_atlas_off_y,	0.0f,				0 );
REG_VAR( r_terrain2_show_atlas_oppa,	1.0f,				0 );

REG_VAR( r_terrain2_show_tiles,			0,					0 );

REG_VAR( r_terrain2_show_splat,			0,					0 );
REG_VAR( r_terrain2_show_splat_idx,		0,					0 );

REG_VAR( r_parabolo_pointlight_shadows,	0,					0 );

REG_VAR( d_terrain2_no_updates,		false,					0 );

REG_VAR( r_show_wind,					0,					0 );
REG_VAR( r_show_probes,					0,					0 );
REG_VAR( r_show_probe_vol_scheme,		1,					0 );
REG_VAR( r_show_probe_boxes,			0,					0 );
REG_VAR( r_show_probes_mode,			0,					0 );
REG_VAR( r_show_probes_radius,			128.0f,				0 );

REG_VAR( r_internal_width,				0,					0 );
REG_VAR( r_internal_height,				0,					0 );
REG_VAR( r_show_luma,					0,					0 );
#endif

REG_VAR( r_light_probes,				0,					0 );
REG_VAR( r_lp_sky_direct,				1.0f,				0 );
REG_VAR( r_lp_sky_bounce,				1.0f,				0 );
REG_VAR( r_lp_sun_bounce,				1.0f,				0 );

REG_VAR( r_lp_show_proximity,			0,					0 );
REG_VAR( r_lp_show_proximity_x,			0,					0 );
REG_VAR( r_lp_show_proximity_y,			0,					0 );
REG_VAR( r_lp_show_proximity_z,			0,					0 );

REG_VAR( r_lp_out_sky,					0.5f,				0 );

#if R3D_ALLOW_TEMPORAL_SSAO
REG_VAR( r_ssao_temporal_reset_freq,	8,					0 );

#define R3D_SSAO_TEMPORAL_FILTER	1
#define R3D_SSAO_TEMPORAL_OPTIMIZE	2

REG_VAR( r_ssao_temporal_filter,		0,					0 );
REG_VAR( r_ssao_temporal_showmask,		0,					0 );
#endif

REG_VAR( r_grass_ssao,					0,					0 );

REG_VAR( r_allow_ingame_unloading,		0,					0 );
REG_VAR( r_need_recalc_probes,			0,					0 );
REG_VAR( r_need_update_probes,			0,					0 );
REG_VAR( r_update_sh_with_sun,			1,					0 );
REG_VAR( r_sky_vis_affects_bounces,		0,					0 );

REG_VAR( r_lp_16bit_amplify,			4,					0 );

#define R3D_HDR_FMT_A16R16G16B16F 1
#define R3D_HDR_FMT_A2R10G10B10 0
REG_VAR( r_hdr_format,					R3D_HDR_FMT_A16R16G16B16F,	1 ) ;

REG_VAR( r_warn_shaders,				0,					0 ) ;
